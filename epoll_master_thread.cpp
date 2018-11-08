#include "epoll_master_tread.h"

EpollMasterThread::EpollMasterThread{
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd_ > 0);
    bzero(&local_address_, sizeof(local_address_)); 

}