#include "epoll_master_tread.h"


int main()
{
    std::string ip("127.0.0.1");
    int port = 1080;
    EpollMasterThread master(ip.c_str(), port);
    
    return 0;
}