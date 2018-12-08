#ifndef EPOLLWORKTHREAD_H
#define EPOLLWORKTHREAD_H
#include "epoll_common.hpp"
#include "client_info.hpp"
#include "memory_pool.hpp"

template<class EventHander, class ClientData>
class EpollWorkThread{
public:
    EpollWorkThread():is_stop_(false),events_(nullptr),client_info_pool_(nullptr){}
    ~EpollWorkThread();
    static void* StartThread(void * instance);
    void Run();

public:
    int work_read_pipe_fd;
    int work_write_pipe_fd;
    int thread_idx;

private:
    void CloseClient(ClientInfo<ClientData> * client_info);
    void FromMaster();
    void OnRead(ClientInfo<ClientData> * client_info);
    void OnWrite(ClientInfo<ClientData> * client_info);
    int Readn(int client_fd, char *buf, size_t size);
    int Writen(int client_fd, char *buf, size_t size);

    virtual void HandAcceptCompleted(ClientInfo<ClientData> * client_info);
    virtual void HandReadCompleted();
    virtual void HandWrite();
    virtual void HandWriteCompleted();
    virtual void HandDisconnect(ClientInfo<ClientData> * client_info);


private:
    int epoll_fd_;
    bool is_stop_;
    std::vector<epoll_event> *events_;
    MemoryPool<ClientInfo<ClientData>> *client_info_pool_;
    EventHander *event_hander_;
};


template<class EventHander, class ClientData>
void *EpollWorkThread::StartThread(void *instance) {
    EpollWorkThread<EventHander, ClientData> * work_thread = (EpollWorkThread<EventHander, ClientData> *)instance;
    work_thread->Run();
    return work_thread;
}

template<class EventHander, class ClientData>
void EpollWorkThread::Run() {
    std::cout<< "work thread:"<< thread_idx <<" is running!" << std::endl;
    client_info_pool_ = new MemoryPool<ClientInfo<ClientData>>();
    event_hander_ = new EventHander();

    epoll_fd_ = epoll_create(5);
    events_ = new std::vector<epoll_event>(64);
    AddFd(epoll_fd_, work_read_pipe_fd, (void *)NULL);
    while(true){
        int event_num = epoll_wait(epoll_fd_, &(*events_->begin()),static_cast<int>(events_->size()), 1);
        if(event_num < 0 && errno != EINTR) {
            std::cout << "epoll failure" << std::endl;
            break;
        }
        if(event_num == events_->size()) {
            events_->resize(events_->size() * 2);
        }

        for(size_t i = 0; i < event_num; i++) {
            ClientInfo<ClientData> * client_info = (ClientInfo<ClientData> *)(*events_)[i].data.ptr;

            if((*events_)[i].events & EPOLLIN) {
                if(client_info == NULL) {
                    FromMaster();
                }else {
                    event_hander_->HaveReadEvent(client_info);
                   // OnRead(client_info);
                }
            }else if((*events_)[i].events & EPOLLOUT) {
                OnWrite(client_info);
            }else if((*events_)[i].events & (EPOLLERR | EPOLLHUP)){
                CloseClient(client_info);
            }
        }
    }
}

template<class EventHander, class ClientData>
void EpollWorkThread::HandAcceptCompleted(ClientInfo<ClientData> * client_info) {

}

template<class EventHander, class ClientData>
void EpollWorkThread::HandReadCompleted() {

}

template<class EventHander, class ClientData>
void EpollWorkThread::OnWrite(ClientInfo<ClientData> * client_info) {

}

template<class EventHander, class ClientData>
void EpollWorkThread::HandWrite(){

}

template<class EventHander, class ClientData>
void EpollWorkThread::HandWriteCompleted(){

}

template<class EventHander, class ClientData>
void EpollWorkThread::HandDisconnect(ClientInfo<ClientData> * client_info) {
    printf("thread:%d,client %d is close!\n", thread_idx, client_info->clinet_fd);
}

template<class EventHander, class ClientData>
void EpollWorkThread::CloseClient(ClientInfo<ClientData> * client_info) {
    RemoveFd(epoll_fd_, client_info->clinet_fd);
    HandDisconnect(client_info);
    client_info_pool_->Delete(client_info);
}

template<class EventHander, class ClientData>
void EpollWorkThread::OnRead(ClientInfo<ClientData> * client_info) {
    char buf[2048];
    int ret = Readn(client_info->clinet_fd, buf, 2048);
    if(ret == -1) {
        CloseClient(client_info);
        return;
    }
    buf[ret] = 0;
    printf(" thread_index: %d, recv = %s\n", thread_idx, buf);
}

template<class EventHander, class ClientData>
int EpollWorkThread::Readn(int client_fd, char * buf, size_t size) {
    size_t have_read_size = 0;
    int ret = 0;
    while(have_read_size < size) {
        ret = read(client_fd, buf + have_read_size, size - have_read_size);
        if(ret <= 0) {
            if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return static_cast<int>(have_read_size);
            } else {
                return -1;
            }
        }
        have_read_size += ret;
    }
    return static_cast<int>(have_read_size);
}

template<class EventHander, class ClientData>
void EpollWorkThread::FromMaster() {
    char buf[4];
    int size = 0;
    int ret = 0;
    while(true) {
        ret = read(work_read_pipe_fd, buf + size, 4 - size);
        if((ret == -1 && errno == EAGAIN) || ret == 0) {
            break;
        }
        size += ret;
        if(size == 4) {
            int client_fd;
            memcpy(&client_fd, buf, 4);
            SetNonblocking(client_fd);

            //TODO add a queue of ClinetInfo
            ClientInfo<ClientData> *client_info = client_info_pool_->New();
            client_info->clinet_fd = client_fd;
            socklen_t peer_len;
            getpeername(client_fd, (struct sockaddr *)&(client_info->client_address), &peer_len);

            AddFd(epoll_fd_, client_fd, (void *)client_info);
            HandAcceptCompleted(client_info);
            size = 0;
        }
    }
}


#endif