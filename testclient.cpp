#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

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
    server.sin_port = htons (9877);
    server.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    
    int res = -1;    
    res = connect( sockfd, (struct sockaddr*)&server, sizeof( server ) );
    if( -1 == res ){
        perror( "sock connect" );
        exit( -1 );
    }
    printf("res = %d\n",res);

    char buf[1024 * 1024];
    memset(buf,1,sizeof(buf));
    res = write(sockfd, buf, 1024);
    printf("write pre\n");
    //sleep(15);
    for(int i = 0; i < 10;i++){
        res = write(sockfd, buf, 1024 * 1024);
        printf("write ok!:%d\n",res);
    }
    
    while(1);

    close( sockfd );

    return 0;
}