#include "epoll_common.hpp"

struct Data {
    int index;
    int client_num;
    struct sockaddr_in server;
};

void StartEpollClient(Data data)
{
    std::vector<int> sock_fds;
    int epoll_fd = epoll_create(5);
    epoll_event events_[65536];
    puts("StartEpollClient");
    for (int i = 0; i < data.client_num; i++) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            perror("sockfd error!");
            throw "sockfd error!";
        }
        int res = connect(sockfd, (struct sockaddr*)&data.server, sizeof(data.server));
        if (res == -1) {
            puts("client connect error!!!");
            close(sockfd);
            continue;
        }
        if (i % 100 == 0)
            printf("client %d connect ok !!\n", i + 1);

        sock_fds.push_back(sockfd);
        long long ptr = sockfd;
        AddFd(epoll_fd, sockfd, (void*)ptr);

        usleep(1000);
    }

    std::string buf = "asdasdasdasdasda";
    char read_buf[1024];
    size_t read_count = 0;
    size_t write_count = 0;
    while (true) {
        sleep(1);
        int index = rand() % sock_fds.size();
        int res = write(sock_fds[index], buf.c_str(), sizeof(buf));
        if (res > 0) {
            printf("write:%d\n", ++write_count);
        }

        int event_num = epoll_wait(epoll_fd, events_, 65536, 1);
        for (int i = 0; i < event_num; i++) {
            int fd = (long long)events_[i].data.ptr;

            if (events_[i].events & EPOLLIN) {
                int ret = read(fd, read_buf, 1024);
                if (ret > 0) {
                    printf("write:%d\n", ++read_count);
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 4) {
        perror("argc < 4!!");
        return -1;
    }
    int client_num;
    client_num = atoi(argv[1]);
    struct sockaddr_in server;
    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[3]));
    inet_pton(AF_INET, argv[2], &server.sin_addr.s_addr);
    Data data;
    data.index = 1;
    data.client_num = client_num;
    data.server = server;
    StartEpollClient(data);

    return 0;
}