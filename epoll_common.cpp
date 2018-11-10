#include "epoll_common.h"


void SetNonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL, new_option);
}

void AddFd(int epoll_fd, int fd, void *ptr){
    epoll_event event;
    event.data.ptr = ptr;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNonblocking(fd);
}

void ModifyFd(int epoll_fd, int fd, uint32_t events, void *ptr) {
    epoll_event event;
    event.data.ptr = ptr;
    event.events = events | EPOLLET | EPOLLRDHUP;
    epoll_ctl( epoll_fd, EPOLL_CTL_MOD, fd, &event );
}

void RemoveFd(int epoll_fd, int fd){
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
}