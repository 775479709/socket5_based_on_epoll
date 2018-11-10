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
    void CloseClient(int client_fd);
    void FromMaster();
    void OnRead(int client_fd);
    void OnWrite(int client_fd);
    int Readn(int client_fd, char *buf, size_t size);
    int Writen(int client_fd, char *buf, size_t size);

    virtual void HandAcceptCompleted(int client_fd);
    virtual void HandReadCompleted();
    virtual void HandWrite();
    virtual void HandWriteCompleted();
    virtual void HandDisconnect(int client_fd);


private:
    int epoll_fd_;
    bool is_stop_;
    std::vector<epoll_event> *events_;
};


#endif