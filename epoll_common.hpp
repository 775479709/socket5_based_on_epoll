#ifndef EPOLLCOMMON_HPP
#define EPOLLCOMMON_HPP
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <vector>
#include <ext/hash_map>
#include <iostream>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include<sys/uio.h>

//#define NDEBUG
#include <assert.h>

void SetNonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL, new_option);
}

void AddFd(int epoll_fd, int fd, void *ptr){
    epoll_event event;
    event.data.ptr = ptr;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNonblocking(fd);
}

void ModifyFd(int epoll_fd, int fd, uint32_t events, void *ptr) {
    epoll_event event;
    event.data.ptr = ptr;
    event.events = events | EPOLLRDHUP;
    epoll_ctl( epoll_fd, EPOLL_CTL_MOD, fd, &event );
}

void RemoveFd(int epoll_fd, int fd){
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
}





#endif