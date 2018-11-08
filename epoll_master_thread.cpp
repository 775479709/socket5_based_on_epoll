#include "epoll_master_tread.h"

EpollMasterThread::EpollMasterThread(const char *ip, int port, size_t work_thread_num = 2){
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd_ > 0);
    bzero(&local_address_, sizeof(local_address_)); 

}