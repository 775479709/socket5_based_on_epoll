#ifndef EPOLLWORKTHREAD_H
#define EPOLLWORKTHREAD_H
#include "epoll_common.h"


class EpollWorkThread{
public:
    EpollWorkThread():is_stop_(false){}
    static void* StartThread(void * instance);
    void Run();

public:
    int work_read_pipe_fd;
    int work_write_pipe_fd;
    int thread_idx;

private:
    void CloseClient(ClientInfo * client_info);
    void FromMaster();
    void OnRead(ClientInfo * client_info);
    void OnWrite(ClientInfo * client_info);
    int Readn(int client_fd, char *buf, size_t size);
    int Writen(int client_fd, char *buf, size_t size);

    virtual void HandAcceptCompleted(ClientInfo * client_info);
    virtual void HandReadCompleted();
    virtual void HandWrite();
    virtual void HandWriteCompleted();
    virtual void HandDisconnect(ClientInfo * client_info);


private:
    int epoll_fd_;
    bool is_stop_;
    std::vector<epoll_event> *events_;
};


#endif