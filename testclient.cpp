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
#include <string>


void SetNonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL, new_option);
}

int main(int argc, char *argv[])
{
    int sockfd = -1;

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( -1 == sockfd ) {
        perror( "sock created" );
        exit( -1 );
    }
    
    struct sockaddr_in server;    
    memset( &server, 0, sizeof( struct sockaddr_in ) );
    server.sin_family = AF_INET;
    server.sin_port = htons (1024);
    server.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    SetNonblocking(sockfd);
    int res = -1;    
    res = connect( sockfd, (struct sockaddr*)&server, sizeof( server ) );
    // if( -1 == res ){
    //     perror( "sock connect" );
    //     exit( -1 );
    // }
    printf("connect:res = %d\n",res);
    int count = 0;
    int len;
    char buf[101000];
    while(1) {
        if(count > 10000)break;
        std::string x = "hello world";
        std::string tmp = std::to_string(++count) + " " + x;
        //if(count == 1)
        res = write(sockfd, tmp.c_str(), tmp.size());
        if(res == -1) {
            if(len == -2) {
                len =read(sockfd, buf, 100000);
                printf("len == -2,len = %d\n",len);
            }
            sleep(1);
        }
        printf("count = %d ,write ok!:%d\n",count,res);
        
        len = -2;
        if(0) {
            len =read(sockfd, buf, 100);
            printf("len = %d\n",len);
            if(len != -1)buf[len] = 0;
            else {
                continue;
            }
        }
        
        //printf("len = %d, data = %s\n",len, buf);
        // if(count > 30000)
        // sleep(1);
        
    }

    close( sockfd );

    return 0;
}