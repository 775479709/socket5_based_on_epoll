#ifndef EPOLLMASTERTHREAD_H
#define EPOLLMASTERTHREAD_H

#include "epoll_common.h"
#include "epoll_work_thread.h"

class WorkThreadInfo{
public:
    size_t client_num;
    int master_read_pipe_fd;
    int master_write_pipe_fd;
    EpollWorkThread *epoll_work_thread;
    WorkThreadInfo() : client_num(0), epoll_work_thread(nullptr){}
};

class EpollMasterThread{
public:
    EpollMasterThread(const char *ip, int port, size_t work_thread_num = 2);
    ~EpollMasterThread();

private:
    void Run();


private:

    int work_thread_num_;
    int epoll_fd_;
    int listen_fd_;
    WorkThreadInfo *work_thread_info_;
    struct sockaddr_in local_address_;
    std::vector<epoll_event> *events_;
    pthread_t *threads_;
};


#endif