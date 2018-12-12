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
    };
public:
    WorkThread();
    ~WorkThread();
    
    void ToBuffer(char *buf, size_t size);
    void DataToClient(ClientInfo *client_info, char *buf, size_t size);
    void DataToClient(ClientInfo *client_info, Buffer *buf);
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
    
private:
    MemoryPool<Buffer, 1024> *buffer_pool_;
    MemoryPool<ClientInfo, 1024> *client_info_pool_;

    struct iovec read_iov_[READ_BUFFER_NUM];
    Buffer *read_iov_head_;

    struct iovec write_iov_[WRITE_BUFFER_NUM];
};

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
    
    if(client_info->read_buffer == nullptr) {
        client_info->read_buffer = read_iov_head_;
        clien_info->read_buffer_size = len;
        for(int i = 0; i < use_buffer_count; i++) {
            if(len - i * BUFFER_SIZE >= BUFFER_SIZE) {
                read_iov_head_->use_size = BUFFER_SIZE;
            } else {
                read_iov_head_->use_size = len - i * BUFFER_SIZE;
            }
            read_iov_head_ = read_iov_head_->next;
        }
    } else {
        Buffer *pre_buf = client_info->read_buffer;
        while(pre_buf->next != nullptr) {
            pre_buf = pre_buf->next;
        }
        if(len <= BUFFER_SIZE - pre_buf->use_size) {
            memcpy(pre_buf->buffer_head + pre_buf->use_size, read_iov_head_->buffer_head, len);
            pre_buf->use_size += len;
            Buffer *temp_buf = read_iov_head_;
            read_iov_head_ = read_iov_head_->next;
            buffer_pool_->Delete(temp_buf);
        }else {
            pre_buf->next = read_iov_head_;
            for(int i = 0; i < use_buffer_count; i++) {
                if(len - i * BUFFER_SIZE >= BUFFER_SIZE) {
                    read_iov_head_->use_size = BUFFER_SIZE;
                } else {
                    read_iov_head_->use_size = len - i * BUFFER_SIZE;
                }
                read_iov_head_ = read_iov_head_->next;
            }
        }
        clien_info->read_buffer_size += len;
    }
    Buffer *pre_buf = nullptr;
    Buffer *head_buf = nullptr;
    for(int i = 0; i < use_buffer_count; i++) {
        BUffer *buffer = buffer_pool_->New();
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

void WorkThread::DataToClient(ClientInfo *client_info, Buffer *buf) {
    if(client_info->write_buffer == nullptr) {
        size_t buf_count = 0;
        for(BUffer *i = buf; i != nullptr && buf_count < WRITE_BUFFER_NUM; i = i->next) {
            write_iov_[buf_count].iov_base = i->buffer_head;
            write_iov_[buf_count].iov_len = i->use_size;
            ++buf_count;
        }
        int len = writev(client_info->client->clinet_fd, write_iov_, buf_count);
        if(len == -1 && errno == EINTR) {
            len = 0;
        } else if(len < 0){
            for(BUffer *i = buf; i != nullptr; i = i->next) {
                buffer_pool_->Delete(i);
            }
            DisconnectClient(client_info);
            return;
        }
        size_t sum_size = 0
        while(sum_size + buf->use_size <= len) {
            sum_size += buf->use_size;
            Buffer *temp = buf;
            buf = buf->next;
            buffer_pool_->Delete(temp);
        }

        if(sum_size < len) {
            size_t size = len - sum_size;
            buf->buffer_head += size;
            //test check
            if(buf->buffer_head > (&buf->buf) + BUFFER  - 1) {
                throw "buf error!!";
            }
            //test check
            buf->use_size -= size;
        }

    }
}



#endif