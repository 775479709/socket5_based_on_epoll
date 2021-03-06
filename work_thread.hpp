#ifndef WORKTHREAD_HPP
#define WORKTHREAD_HPP
#include "epoll_work_thread.hpp"

#define BUFFER_SIZE 4096
#define READ_BUFFER_NUM 16
#define WRITE_BUFFER_NUM 16

class WorkThread : public EpollWorkThread {
public:
    struct Buffer {
        Buffer* next;
        char* buffer_head;
        size_t use_size;
        char buf[BUFFER_SIZE];
        Buffer()
            : next(nullptr)
            , buffer_head(buf)
            , use_size(0)
        {
        }
    };

    struct ClientInfo {
        Client* client;
        Buffer* read_buffer;
        Buffer* write_buffer;
        size_t read_buffer_size;
        size_t write_buffer_size;

        ClientInfo()
            : client(nullptr)
            , read_buffer(nullptr)
            , write_buffer(nullptr)
            , read_buffer_size(0)
            , write_buffer_size(0)
        {
        }
        Buffer* GetBackBuffer(bool is_write_buffer);
        void ResetBuffer(bool is_write_buffer);
    };

public:
    WorkThread();
    ~WorkThread();

    Buffer* ToBuffer(const char* buf, size_t size);
    void CleanClientBuffer(ClientInfo* client_info, bool is_write_buffer);
    bool DataToClient(ClientInfo* client_info, const char* buf, size_t size);
    bool DataToClient(ClientInfo* client_info, Buffer* buffer, size_t size);
    void DisconnectClient(ClientInfo* client_info);

    virtual void NewClient(ClientInfo* client_info) = 0;
    virtual void DataFromClient(ClientInfo* client_info) = 0;
    virtual void DataToClientCompleted(ClientInfo* client_info) = 0;
    virtual void ClientLeave(ClientInfo* client_info) = 0;

private:
    void HandAcceptCompleted(Client* client);
    void HandReadEvent(Client* client);
    void HandWriteEvent(Client* client);
    void HandDisconnect(Client* client);

    bool Writev(ClientInfo* client_info);

    void MergeBuffer(Buffer* buffer);
    void AddBufferBack(ClientInfo* client_info, bool is_write_buffer, Buffer* buffer, size_t size);

private:
    MemoryPool<Buffer, 1024>* buffer_pool_;
    MemoryPool<ClientInfo, 1024>* client_info_pool_;

    struct iovec read_iov_[READ_BUFFER_NUM];
    Buffer* read_iov_head_;

    struct iovec write_iov_[WRITE_BUFFER_NUM];
};

WorkThread::Buffer* WorkThread::ClientInfo::GetBackBuffer(bool is_write_buffer)
{
    Buffer* buffer = (is_write_buffer ? write_buffer : read_buffer);
    if (buffer == nullptr) {
        return nullptr;
    }
    while (buffer->next != nullptr) {
        buffer = buffer->next;
    }
    return buffer;
}

void WorkThread::ClientInfo::ResetBuffer(bool is_write_buffer)
{
    if (!is_write_buffer) {
        read_buffer = nullptr;
        read_buffer_size = 0;
    } else {
        write_buffer = nullptr;
        write_buffer_size = 0;
    }
}

void WorkThread::CleanClientBuffer(ClientInfo* client_info, bool is_write_buffer)
{
    Buffer*& buffer = is_write_buffer ? client_info->write_buffer : client_info->read_buffer;
    while (buffer != nullptr) {
        Buffer* temp = buffer;
        buffer = buffer->next;
        buffer_pool_->Delete(temp);
    }
    (is_write_buffer ? client_info->write_buffer_size : client_info->read_buffer_size) = 0;
}

WorkThread::WorkThread()
{
    buffer_pool_ = new MemoryPool<Buffer, 1024>();
    client_info_pool_ = new MemoryPool<ClientInfo, 1024>();
    Buffer* pre_buffer = nullptr;
    for (int i = 0; i < READ_BUFFER_NUM; i++) {
        Buffer* buffer = buffer_pool_->New();
        if (i == 0) {
            read_iov_head_ = buffer;
        } else {
            pre_buffer->next = buffer;
        }
        pre_buffer = buffer;
        read_iov_[i].iov_base = (char*)buffer->buffer_head;
        read_iov_[i].iov_len = BUFFER_SIZE;
    }
}

WorkThread::~WorkThread()
{

    while (read_iov_head_ != nullptr) {
        Buffer* buf = read_iov_head_;
        read_iov_head_ = read_iov_head_->next;
        buffer_pool_->Delete(buf);
    }

    delete buffer_pool_;
    delete client_info_pool_;
}

void WorkThread::MergeBuffer(Buffer* buffer)
{
    if (buffer == nullptr) {
        return;
    }
    if (buffer->next != nullptr && BUFFER_SIZE - buffer->use_size >= buffer->next->use_size) {
        memcpy(buffer->buffer_head + buffer->use_size, buffer->next->buffer_head, buffer->next->use_size);
        buffer->use_size += buffer->next->use_size;
        Buffer* next_buffer = buffer->next->next;
        buffer_pool_->Delete(buffer->next);
        buffer->next = next_buffer;
    }
}

void WorkThread::AddBufferBack(ClientInfo* client_info, bool is_write_buffer, Buffer* buffer, size_t size)
{
    Buffer*& client_buffer = (is_write_buffer ? client_info->write_buffer : client_info->read_buffer);
    size_t& client_buffer_size = (is_write_buffer ? client_info->write_buffer_size : client_info->read_buffer_size);
    if (client_buffer == nullptr) {
        client_buffer = buffer;
    } else {
        Buffer* back_buffer = client_info->GetBackBuffer(is_write_buffer);
        back_buffer->next = buffer;
        MergeBuffer(back_buffer);
    }
    client_buffer_size += size;
    //printf("AddBufferBack :size =%d\n",client_buffer_size);
}

void WorkThread::HandAcceptCompleted(Client* client)
{
    //test
    //puts("HandAcceptCompleted::");
    //test
    ClientInfo* client_info = client_info_pool_->New();
    client_info->client = client;
    client->data = (void*)client_info;
    NewClient(client_info);
}

void WorkThread::HandReadEvent(Client* client)
{
    //test
    //puts("HandReadEvent::");
    //test

    int len = readv(client->clinet_fd, read_iov_, READ_BUFFER_NUM);

    //test
    //printf("rev len = %d\n", len);
    // for(int i = 0; i < 16;i++) {
    //     printf("%d : %u\n",i,read_iov_[i].iov_len);
    // }
    //test

    ClientInfo* client_info = (ClientInfo*)client->data;
    if (len <= 0) {
        // len == 0 have been closed
        DisconnectClient(client_info);
        return;
    }
    size_t use_buffer_count = len / BUFFER_SIZE + (len % BUFFER_SIZE != 0);

    //printf("use_buffer_count = %u\n",use_buffer_count);

    Buffer* remaining_buffer = read_iov_head_;
    Buffer* last_remaining_buffer = nullptr;
    for (int i = 0; i < use_buffer_count; i++) {
        if (len - i * BUFFER_SIZE > BUFFER_SIZE) {
            remaining_buffer->use_size = BUFFER_SIZE;
        } else {
            remaining_buffer->use_size = len - i * BUFFER_SIZE;
            last_remaining_buffer = remaining_buffer;
        }
        remaining_buffer = remaining_buffer->next;
    }
    last_remaining_buffer->next = nullptr;

    AddBufferBack(client_info, 0, read_iov_head_, len);

    Buffer* pre_buf = nullptr;
    Buffer* head_buf = nullptr;
    for (int i = 0; i < use_buffer_count; i++) {
        Buffer* buffer = buffer_pool_->New();
        if (i == 0) {
            head_buf = buffer;
        } else {
            pre_buf->next = buffer;
        }
        pre_buf = buffer;
        read_iov_[i].iov_base = (char*)buffer->buffer_head;
        read_iov_[i].iov_len = BUFFER_SIZE;
    }

    pre_buf->next = remaining_buffer;
    read_iov_head_ = head_buf;
    if (len == BUFFER_SIZE * READ_BUFFER_NUM) {
        ModifyFd(epoll_fd, client_info->client->clinet_fd, EPOLLIN, (void*)client_info->client);
    }
    DataFromClient(client_info);
}

void WorkThread::HandWriteEvent(Client* client)
{
    //printf("HandWrite event\n");

    ClientInfo* client_info = (ClientInfo*)(client->data);
    if (Writev(client_info)) {
        DeleteClientEvent(client_info->client, EPOLLOUT);
        //ModifyFd(epoll_fd, client_info->client->clinet_fd, EPOLLIN, (void *)client_info->client);
        DataToClientCompleted(client_info);
    }
}

bool WorkThread::Writev(ClientInfo* client_info)
{
    //printf("Writev::\n");

    Buffer* buffer = client_info->write_buffer;
    size_t size = client_info->write_buffer_size;

    //printf("writev::size = %d\n", size);
    fflush(stdout);

    if (size != 0 && buffer == nullptr) {
        perror("1.Writev:: buffer | size error!!");
        throw "1.Writev:: buffer | size error!!";
    }

    //printf("asd = %d\n",size);
    fflush(stdout);

    if (buffer == nullptr) {
        return true;
    }
    size_t count = 0;

    while (count < WRITE_BUFFER_NUM && buffer != nullptr && buffer->use_size <= size) {
        write_iov_[count].iov_base = buffer->buffer_head;
        write_iov_[count].iov_len = buffer->use_size;
        size -= buffer->use_size;
        buffer = buffer->next;
        count++;
        //test check
        if (size != 0 && buffer == nullptr) {
            printf("count = %u, size = %u\n", count, size);
            perror("2.Writev:: buffer | size error!!");
            throw "2.Writev:: buffer | size error!!";
        }
        //test check
    }

    //test check
    if (size != 0 && count < WRITE_BUFFER_NUM) {
        perror("3.Writev:: buffer | size error!!");
        throw "3.Writev:: buffer | size error!!!!!!!";
    }
    //test check

    //printf("pre write\n");
    //fflush(stdout);

    int len = writev(client_info->client->clinet_fd, write_iov_, count);

    //printf("writev len = %d\n", len);

    if (len == -1 && (errno == EINTR || errno == EAGAIN)) {
        return false;
    } else if (len < 0) {
        for (Buffer* i = buffer; i != nullptr; i = buffer) {
            buffer = buffer->next;
            buffer_pool_->Delete(i);
        }
        client_info->ResetBuffer(1);
        DisconnectClient(client_info);
        return false;
    }
    buffer = client_info->write_buffer;
    size_t sum_size = 0;
    while (buffer != nullptr && sum_size + buffer->use_size <= len) {
        sum_size += buffer->use_size;
        Buffer* temp = buffer;
        buffer = buffer->next;
        buffer_pool_->Delete(temp);
    }

    if (sum_size < len) {
        size_t size = len - sum_size;
        buffer->buffer_head += size;
        //test check
        if (buffer->buffer_head > buffer->buf + BUFFER_SIZE - 1) {
            throw "DataToClient::buf error!!";
        }
        //test check
        buffer->use_size -= size;
    }
    size_t all_buffer_size = client_info->write_buffer_size;
    client_info->ResetBuffer(1);
    if (len < all_buffer_size) {
        AddBufferBack(client_info, 1, buffer, all_buffer_size - len);
        //puts("len < size");
        //ModifyFd(epoll_fd, client_info->client->clinet_fd, EPOLLOUT, (void *)client_info->client);
        return false;
    } else {
        //ModifyFd(epoll_fd, client_info->client->clinet_fd, EPOLLIN, (void *)client_info->client);
        //puts("return true");
        return true;
    }
}

bool WorkThread::DataToClient(ClientInfo* client_info, Buffer* buffer, size_t size)
{
    //printf("data to client :buffer\n");
    bool is_empty = client_info->write_buffer == nullptr;
    AddBufferBack(client_info, 1, buffer, size);
    if (is_empty) {
        if (Writev(client_info)) {
            DataToClientCompleted(client_info);
            return true;
        } else {
            AddClientEvent(client_info->client, EPOLLOUT);
            //ModifyFd(epoll_fd, client_info->client->clinet_fd, EPOLLOUT, (void *)client_info->client);
        }
    }
    return false;
}

WorkThread::Buffer* WorkThread::ToBuffer(const char* buf, size_t size)
{
    //printf("ToBuffer:: size = %u\n",size);
    if (size == 0) {
        return nullptr;
    }
    Buffer* head = nullptr;
    Buffer* pre_buffer = nullptr;
    size_t cpy_size = 0;
    while (cpy_size < size) {
        Buffer* buffer = buffer_pool_->New();
        if (head == nullptr) {
            head = buffer;
        } else {
            pre_buffer->next = buffer;
        }
        pre_buffer = buffer;
        if (size - cpy_size > BUFFER_SIZE) {
            memcpy(buffer->buffer_head, buf + cpy_size, BUFFER_SIZE);
            buffer->use_size = BUFFER_SIZE;
            cpy_size += BUFFER_SIZE;
        } else {
            memcpy(buffer->buffer_head, buf + cpy_size, size - cpy_size);
            buffer->use_size = size - cpy_size;
            cpy_size = size;
        }
        //printf("ToBuffer::cpy_size = %u\n",cpy_size);
    }
    return head;
}

bool WorkThread::DataToClient(ClientInfo* client_info, const char* buf, size_t size)
{
    // if(size > WRITE_BUFFER_NUM * BUFFER_SIZE) {
    //     perror("DataToClient:: size is too large!!");
    //     throw "DataToClient:: size is too large!!";
    // }
    Buffer* buffer = ToBuffer(buf, size);
    return DataToClient(client_info, buffer, size);
}

void WorkThread::DisconnectClient(ClientInfo* client_info)
{
    //printf("dis connnect\n");
    CloseClient(client_info->client);
    CleanClientBuffer(client_info, 0);
    CleanClientBuffer(client_info, 1);
    client_info_pool_->Delete(client_info);
}

void WorkThread::HandDisconnect(Client* client)
{
    ClientLeave((ClientInfo*)client->data);
}

#endif