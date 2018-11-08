#ifndef EPOLLWORKTHREAD_H
#define EPOLLWORKTHREAD_H
#include "epoll_common.h"


class EpollWorkThread{
public:
    EpollWorkThread():is_stop_(false){}

public:
    int master_fd;
    int work_fd;
    int epoll_fd;
    int thread_idx;
private:
    bool is_stop_;
};


#endif