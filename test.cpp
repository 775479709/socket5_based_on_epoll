#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include<stdio.h>
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

#define    MAXLINE        4096
#define    LISTENQ        20
#define    SERV_PORT    1023

void SetNonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL, new_option);
}

int main(int argc, char* argv[])
{
    int i, maxi, listenfd, connfd, sockfd, epfd,nfds;
    ssize_t n;
    char BUF[MAXLINE];
    socklen_t clilen;

    //ev用于注册事件,数组用于回传要处理的事件

    struct epoll_event ev,events[20];
    //生成用于处理accept的epoll专用的文件描述符

    epfd=epoll_create(256);
    struct sockaddr_in cliaddr, servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
 
    //setnonblocking(listenfd);
    //设置与要处理的事件相关的文件描述符

    ev.data.fd=listenfd;
    ev.events=EPOLLIN|EPOLLET;
    //注册epoll事件

    epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port = htons (SERV_PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    maxi = 0; //useless
    size_t all_rev_size = 0;
    for ( ; ; ) {
        nfds=epoll_wait(epfd,events,20,0);

        for(i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            {
                connfd = accept(listenfd,(struct sockaddr *)&cliaddr, &clilen);
                if(connfd<0){
                    perror("connfd<0");
                }
                SetNonblocking(connfd);

                char *str = inet_ntoa(cliaddr.sin_addr);
                printf("accapt a connection from %s\n", str);

                ev.data.fd=connfd;
                ev.events=EPOLLIN|EPOLLET| EPOLLRDHUP;
                sleep(10);
                printf("epoll _ctl epoll in!\n");
                fflush(stdout);
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
            else if (events[i].events & EPOLLRDHUP) {
                epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, 0);
                printf("delelte fd\n");
            }
            else if (events[i].events&EPOLLIN)
            //如果是已经连接的用户，并且收到数据，那么进行读入。

            {
                printf("EPOLLIN\n");
                struct iovec iov[32];
                char *buf[32];
            
                for(int ii = 0; ii < 32;ii++) {
                    buf[ii] = new char[1024 * 1024];
                    iov[ii].iov_base = (void*)buf[ii];
                    iov[ii].iov_len =  1024 * 1024;
                }
                int len = readv(events[i].data.fd,iov, 32);
                if(len == -1){
                    printf("readv error!\n");
                }

                all_rev_size += len;
                 printf("all_rev_size = %lu\n",all_rev_size);
                 printf("iov=%d\n",iov[0].iov_len);
                 sleep(5);

                // ev.data.fd=connfd;
                // ev.events=EPOLLIN|EPOLLET;
                // printf("epoll _ctl epoll in!\n");
                //epoll_ctl(epfd,EPOLL_CTL_MOD,connfd,&ev);

                // while(true) {
                //     len = readv(events[i].data.fd,iov, 32);
                //     if(len > 0) {

                //         all_rev_size +=len;
                //         printf("loop:all_rev_size = %lu\n",all_rev_size);
                //     }
                // }
               


                // if ( (sockfd = events[i].data.fd) < 0)
                //     continue;
                // if ( (n = read(sockfd, BUF, MAXLINE)) < 0) {
                //     if (errno == ECONNRESET) {
                //         close(sockfd);
                //         events[i].data.fd = -1;
                //     } else
                //         printf("readline error\n");
                // } else if (n == 0) {
                //     close(sockfd);
                //     events[i].data.fd = -1;
                // }
                // BUF[n] = '\0';
                // printf("AFTER EPOLLIN\n");

                // ev.data.fd=sockfd;
                // ev.events=EPOLLOUT|EPOLLET;
                // //读完后准备写
                // epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

            } 
            // else if(events[i].events&EPOLLOUT) // 如果有数据发送

            // {
            //     sockfd = events[i].data.fd;
            //     write(sockfd, BUF, n);

            //     ev.data.fd=sockfd;
            //     ev.events=EPOLLIN|EPOLLET;
            //     //写完后，这个sockfd准备读
            //     epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            // }
        }
    }
    return 0;
}