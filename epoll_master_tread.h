#ifndef EPOLLMASTERTHREAD_H
#define EPOLLMASTERTHREAD_H

#include "epoll_common.h"
#include "epoll_work_thread.h"

class WorkThreadInfo{
public:
    size_t client_num;
    EpollWorkThread *epoll_work_thread;
};

class EpollMasterThread{
public:
    EpollMasterThread(const char *ip, int port, size_t work_thread_num = 2);
    ~EpollMasterThread();

private:
    int epoll_fd_;
    int listen_fd_;
    WorkThreadInfo *work_thread_info_;
    struct sockaddr_in local_address_;
};


#endif