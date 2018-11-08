#ifndef EPOLLPROCESSPOOL_H
#define EPOLLPROCESSPOOL_H

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
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
//#define NDEBUG
#include <assert.h>

// 描述一个子进程类
class process {
public:
    process() : pid( -1 ){}

public:
    // 进程ID
    pid_t pid;
    // 管道句柄
    int pipe_fd[2];
};





#endif
