#ifndef EPOLLCOMMON_H
#define EPOLLCOMMON_H
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

//#define NDEBUG
#include <assert.h>

void SetNonblocking(int fd);

void AddFd(int epoll_fd, int fd, void *ptr);

void ModifyFd(int epoll_fd, int fd, uint32_t events, void *ptr);

void RemoveFd(int epoll_fd, int fd);

class ClientInfo{

public:
    struct sockaddr_in client_address;
    int clinet_fd;
};






#endif