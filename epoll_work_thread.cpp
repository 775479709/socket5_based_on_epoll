#include "epoll_work_thread.h"

void *EpollWorkThread::StartThread(void *instance) {
    EpollWorkThread * work_thread = (EpollWorkThread *)instance;
    work_thread->Run();
    return work_thread;
}

void EpollWorkThread::Run() {
    
}