#include "epoll_master_tread.hpp"
#include "work_thread.hpp"


class ServerWorkThread : public WorkThread{
public:
    int client_count = 0;
    ServerWorkThread() {
    }
    void NewClient(ClientInfo *client_info){
        printf("have client :%d, now client_count = %d\n", client_info->client->clinet_fd, ++client_count);
    }
    void DataFromClient(ClientInfo *client_info){
        //printf("client_read_buffer_size= %d\n",client_info->read_buffer_size);

        char *buf = new char[client_info->read_buffer_size];
        size_t size = 0;
        for(auto *head = client_info->read_buffer; head != nullptr; head = head->next) {
            memcpy(buf + size, head->buffer_head, head->use_size);
            size += head->use_size;
        }
        CleanClientBuffer(client_info, 0);
        //puts("DataFromClient::send data to client");
        bool is = DataToClient(client_info, buf, size);
        delete buf;
        //printf("count = %d,DataFromClient::send data to client ok!!!\n",count++);
    }
    void DataToClientCompleted(ClientInfo *client_info){
        //printf("send to Client %d is completed\n",client_info->client->clinet_fd);
    }
    void ClientLeave(ClientInfo *client_info){
        printf("client %d is leaved, now_client_count = %d\n",client_info->client->clinet_fd,--client_count);
    }
};



int main()
{
    EpollMasterThread<ServerWorkThread> *server = new EpollMasterThread<ServerWorkThread>("127.0.0.1", 1024);
    delete server;
    
    return 0;
}