#include "epoll_master_tread.hpp"
#include "work_thread.hpp"

std::string data;

class ServerWorkThread : public WorkThread {
public:
    int client_count = 0;
    ServerWorkThread()
    {
    }
    void NewClient(ClientInfo* client_info)
    {
        if (++client_count % 1000 == 0)
            printf("have client :%d, now client_count = %d\n", client_info->client->clinet_fd, client_count);
    }
    void DataFromClient(ClientInfo* client_info)
    {
        //printf("client_read_buffer_size= %d\n",client_info->read_buffer_size);

        char* buf = new char[client_info->read_buffer_size + 1];
        size_t size = 0;
        for (auto* head = client_info->read_buffer; head != nullptr; head = head->next) {
            memcpy(buf + size, head->buffer_head, head->use_size);
            size += head->use_size;
        }
        buf[client_info->read_buffer_size] = 0;
        //printf("data from client : %s\n", buf);
        CleanClientBuffer(client_info, 0);
        //puts("DataFromClient::send data to client");
        if (client_info->write_buffer_size < 10 * 1024 * 1024) {
            bool is = DataToClient(client_info, data.c_str(), data.size());
            //printf("is = %d\n", is);
        }
        delete buf;
        //printf("count = %d,DataFromClient::send data to client ok!!!\n",count++);
    }
    void DataToClientCompleted(ClientInfo* client_info)
    {
        //printf("send to Client %d is completed\n",client_info->client->clinet_fd);
    }
    void ClientLeave(ClientInfo* client_info)
    {
        if (--client_count % 1000 == 0)
            printf("client %d is leaved, now_client_count = %d\n", client_info->client->clinet_fd, client_count);
    }
};

int main()
{
    freopen("data.txt", "r", stdin);
    int c;
    while ((c = getchar()) != -1) {
        data.push_back(c);
    }
    std::cout << "data:" << data;

    signal(SIGPIPE, SIG_IGN);
    EpollMasterThread<ServerWorkThread>* server = new EpollMasterThread<ServerWorkThread>(0, 1180);
    delete server;

    return 0;
}