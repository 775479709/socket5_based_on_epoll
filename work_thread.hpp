#ifndef WORKTHREAD_HPP
#define WORKTHREAD_HPP
#include "epoll_work_thread.hpp"

#define BUFFER_SIZE 4096
#define READ_BUFFER_NUM 16
#define WRITE_BUFFER_NUM 16


class WorkThread : public EpollWorkThread {
public:

    struct Buffer{
        Buffer *next;
        char *buffer_head;
        size_t use_size;
        char buf[BUFFER_SIZE];
        Buffer():next(nullptr),buffer_head(buf),use_size(0){}
    }; 

    struct ClientInfo{
        Client *client;
        Buffer *read_buffer;
        Buffer *write_buffer;
        size_t read_buffer_size;
        size_t write_buffer_size;

        ClientInfo():client(nullptr),read_buffer(nullptr),write_buffer(nullptr),read_buffer_size(0),write_buffer_size(0){}
        Buffer *GetBackBuffer(bool is_write_buffer);
    };
public:
    WorkThread();
    ~WorkThread();
    
    void ToBuffer(char *buf, size_t size);
    void DataToClient(ClientInfo *client_info, char *buf, size_t size);
    void DataToClient(ClientInfo *client_info, Buffer *buffer, size_t size);
    void DisconnectClient(ClientInfo *client_info);

    virtual void NewClient(ClientInfo *client_info) = 0;
    virtual void DataFromClient(ClientInfo *client_info) = 0;
    virtual void DataToClientCompleted(ClientInfo *client_info) = 0;
    virtual void ClientLeave(ClientInfo *client_info) = 0;
    


private:
    void HandAcceptCompleted(Client *client);
    void HandReadEvent(Client *client);
    void HandWriteEvent(Client *client);
    void HandDisconnect(Client *client);

    bool Writev(ClientInfo *client_info);
    
    void MergeBuffer(Buffer *buffer);
    void AddBufferBack(ClientInfo *client_info, bool is_write_buffer, Buffer * buffer, size_t size);
    
private:
    MemoryPool<Buffer, 1024> *buffer_pool_;
    MemoryPool<ClientInfo, 1024> *client_info_pool_;

    struct iovec read_iov_[READ_BUFFER_NUM];
    Buffer *read_iov_head_;

    struct iovec write_iov_[WRITE_BUFFER_NUM];
};

WorkThread::Buffer *WorkThread::ClientInfo::GetBackBuffer(bool is_write_buffer) {
    Buffer *buffer = is_write_buffer ? write_buffer : read_buffer;
    if(buffer == nullptr) {
        return nullptr;
    }
    while(buffer->next != nullptr) {
        buffer = buffer->next;
    }
    return buffer;
}



WorkThread::WorkThread(){
    buffer_pool_ = new MemoryPool<Buffer, 1024>();
    client_info_pool_ = new MemoryPool<ClientInfo, 1024>();
    Buffer *pre_buffer = nullptr;
    for(int i = 0; i < READ_BUFFER_NUM; i++) {
        Buffer *buffer = buffer_pool_->New();
        if(i == 0) {
            read_iov_head_ = buffer;
        }else {
            pre_buffer->next = buffer;
        }
        pre_buffer = buffer;
        read_iov_[i].iov_base = buffer->buffer_head;
        read_iov_[i].iov_len = BUFFER_SIZE;
    }
}

WorkThread::~WorkThread(){

    while(read_iov_head_ != nullptr) {
        Buffer *buf = read_iov_head_;
        read_iov_head_ = read_iov_head_->next;
        buffer_pool_->Delete(buf);
    }

    delete buffer_pool_;
    delete client_info_pool_;
}


void WorkThread::MergeBuffer(Buffer *buffer) {
    if(buffer == nullptr) {
        return;
    }
    if(buffer->next != nullptr && BUFFER_SIZE - buffer->use_size >= buffer->next->use_size) {
        memcpy(buffer->buffer_head + buffer->use_size, buffer->next->buffer_head, buffer->next->use_size);
        buffer->use_size += buffer->next->use_size;
        Buffer *next_buffer = buffer->next->next;
        buffer_pool_->Delete(buffer->next);
        buffer->next = next_buffer;
    }
}

void WorkThread::AddBufferBack(ClientInfo *client_info, bool is_write_buffer, Buffer * buffer, size_t size) {
    Buffer *client_buffer = is_write_buffer ? client_info->write_buffer : client_info->read_buffer;
    size_t &client_buffer_size = is_write_buffer ? client_info->write_buffer_size : client_info->read_buffer_size;

    if(client_buffer == nullptr) {
        client_buffer = buffer;
    } else {
        Buffer *back_buffer = client_info->GetBackBuffer(is_write_buffer);
        back_buffer->next = buffer;
        MergeBuffer(back_buffer);
    }
    client_buffer_size += size;
}

void WorkThread::HandAcceptCompleted(Client *client) {
    ClientInfo *client_info = client_info_pool_->New();
    client_info->client = client;
    client->data = (void *)client_info;
    NewClient(client_info);
}

void WorkThread::HandReadEvent(Client *client) {
    int len = readv(client->clinet_fd, read_iov_, READ_BUFFER_NUM);
    ClientInfo *client_info = (ClientInfo *)client->data;
    if(len <= 0) {
        DisconnectClient(client_info);
        return;
    }
    size_t use_buffer_count = len / BUFFER_SIZE + (len % BUFFER_SIZE != 0);

    for(int i = 0; i < use_buffer_count; i++) {
        if(len - i * BUFFER_SIZE >= BUFFER_SIZE) {
            read_iov_head_->use_size = BUFFER_SIZE;
        } else {
            read_iov_head_->use_size = len - i * BUFFER_SIZE;
        }
        read_iov_head_ = read_iov_head_->next;
    }
    AddBufferBack(client_info, 0, read_iov_head_, len);

    Buffer *pre_buf = nullptr;
    Buffer *head_buf = nullptr;
    for(int i = 0; i < use_buffer_count; i++) {
        Buffer *buffer = buffer_pool_->New();
        if(i == 0) {
            head_buf = buffer;
        } else {
            pre_buf->next = buffer;
        }
        pre_buf = buffer;
        read_iov_[i].iov_base = buffer->buffer_head;
        read_iov_[i].iov_len = BUFFER_SIZE;
    }
    pre_buf->next = read_iov_head_;
    read_iov_head_ = head_buf;
    DataFromClient(client_info);
}


void WorkThread::HandWriteEvent(Client *client) {
    ClientInfo *client_info = (ClientInfo *)(client->data);
    if(Writev(client_info)) {
        DataToClientCompleted(client_info);
    }
}


bool WorkThread::Writev(ClientInfo *client_info){
    int i;
    Buffer *buffer = client_info->write_buffer;
    size_t size = client_info->write_buffer_size;
    client_info->write_buffer = nullptr;
    client_info->write_buffer_size = 0;
    for(i = 0; i < WRITE_BUFFER_NUM && buffer->use_size <= size; i++) {
        write_iov_[i].iov_base = buffer->buffer_head;
        write_iov_[i].iov_len = buffer->use_size;
        size -= buffer->use_size;
        buffer = buffer->next;
        //test check
        if(size && buffer == nullptr) {
            throw "1.Writev:: buffer | size error!!";
        }
        //test check
    }
    //test check
    if(size != 0 && i < WRITE_BUFFER_NUM) {
        throw "2.Writev:: buffer | size error!!!!!!!";
    }
    //test check
    int len = writev(client_info->client->clinet_fd, write_iov_, i);
    if(len == -1 && errno == EINTR) {
        len = 0;
    } else if(len < 0){
        for(Buffer *i = buffer; i != nullptr; i = buffer) {
            buffer = buffer->next;
            buffer_pool_->Delete(i);
        }
        DisconnectClient(client_info);
        return;
    }
    size_t sum_size = 0;
    while(sum_size + buffer->use_size <= len) {
        sum_size += buffer->use_size;
        Buffer *temp = buffer;
        buffer = buffer->next;
        buffer_pool_->Delete(temp);
    }

    if(sum_size < len) {
        size_t size = len - sum_size;
        buffer->buffer_head += size;
        //test check
        if(buffer->buffer_head > buffer->buf + BUFFER_SIZE  - 1) {
            throw "DataToClient::buf error!!";
        }
        //test check
        buffer->use_size -= size;
    }
    if(len < size) {
        AddBufferBack(client_info, 1, buffer, size - len);
        ModifyFd(epoll_fd, client_info->client->clinet_fd, EPOLLOUT, (void *)client_info->client);
        return false;
    } else {
        ModifyFd(epoll_fd, client_info->client->clinet_fd, EPOLLIN, (void *)client_info->client);
        return true;
    }
} 

void WorkThread::DataToClient(ClientInfo *client_info, Buffer *buffer, size_t size) {
    bool is_empty = client_info->write_buffer == nullptr;
    AddBufferBack(client_info, 1, buffer, size);
    if(is_empty) {
        Writev(client_info);
    }
}



#endif