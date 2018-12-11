#ifndef WORKTHREAD_HPP
#define WORKTHREAD_HPP
#include "epoll_work_thread.hpp"

#define BUFFER_SIZE 4096
#define READ_BUFFER_NUM 16


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
    virtual void DataToClientComplete(ClientInfo *client_info) = 0;
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
    Buffer *read_iov_head;

};

WorkThread::WorkThread(){
    buffer_pool_ = new MemoryPool<Buffer, 1024>();
    client_info_pool_ = new MemoryPool<ClientInfo, 1024>();
    Buffer *pre_buffer = nullptr;
    for(int i = 0; i < READ_BUFFER_NUM; i++) {
        Buffer *buffer = buffer_pool_->New();
        if(i == 0) {
            read_iov_head = buffer;
        }else {
            pre_buffer->next = buffer;
        }
        pre_buffer = buffer;
        read_iov_[i].iov_base = buffer->buffer_head;
        read_iov_[i].iov_len = BUFFER_SIZE;
    }
}

WorkThread::~WorkThread(){

    while(read_iov_head != nullptr) {
        Buffer *buf = read_iov_head;
        read_iov_head = read_iov_head->next;
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
    if(client_info->read_buffer_size == 0) {
        client_info->read_buffer = read_iov_head;
        
    }
    Buffer *pre_buf = client_info->read_buffer;


}


#endif