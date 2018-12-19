#ifndef EPOLLCOMMON_HPP
#define EPOLLCOMMON_HPP
#include <arpa/inet.h>
#include <errno.h>
#include <ext/hash_map>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

//#define NDEBUG
#include <assert.h>

void SetNonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

void AddFd(int epoll_fd, int fd, void* ptr)
{
    epoll_event event;
    event.data.ptr = ptr;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    SetNonblocking(fd);
}

void ModifyFd(int epoll_fd, int fd, uint32_t events, void* ptr)
{
    epoll_event event;
    event.data.ptr = ptr;
    event.events = events;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void RemoveFd(int epoll_fd, int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
}

#endif