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
    int epoll_fd_;
    bool is_stop_;
    std::vector<epoll_event> *events_;
};


#endif