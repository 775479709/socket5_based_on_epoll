#ifndef CLIENTINFO
#define CLIENTINFO

#include "epoll_common.hpp"

template<class T>
class ClientInfo{

public:
    struct sockaddr_in client_address;
    int clinet_fd;
    T data;
};

#endif