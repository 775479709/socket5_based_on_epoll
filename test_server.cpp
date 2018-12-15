#include "epoll_master_tread.hpp"
#include "work_thread.hpp"


class ServerWorkThread : public WorkThread{
public:
    int count = 0;
    ServerWorkThread() {
    }
    void NewClient(ClientInfo *client_info){
        printf("have client :%d\n", client_info->client->clinet_fd);
    }
    void DataFromClient(ClientInfo *client_info){
        printf("client_read_buffer_size= %d\n",client_info->read_buffer_size);
        fflush(stdout);
        //client_info->read_buffer->buf[client_info->read_buffer_size] = 0;
        //printf("data from client: %d, data = %s\n",client_info->client->clinet_fd, client_info->read_buffer->buf);
        
        char *buf = new char[client_info->read_buffer_size];
        size_t size = 0;
        for(auto *head = client_info->read_buffer; head != nullptr; head = head->next) {
            // if(size + head->use_size <= 65535) {
                
            // }else {
            //     memcpy(buf + size, head->buffer_head, 65536 - size);
            //     size = 65535;
            // }
            memcpy(buf + size, head->buffer_head, head->use_size);
            size += head->use_size;
            
        }
        CleanClientBuffer(client_info, 0);
        puts("DataFromClient::send data to client");
        DataToClient(client_info, buf, size);
        delete buf;
        printf("count = %d,DataFromClient::send data to client ok!!!\n",count++);
        fflush(stdout);

        sleep(1);
    }
    void DataToClientCompleted(ClientInfo *client_info){
        printf("send to Client %d is completed\n",client_info->client->clinet_fd);
    }
    void ClientLeave(ClientInfo *client_info){
        printf("client %d is leaved\n",client_info->client->clinet_fd);
    }
};



int main()
{
    EpollMasterThread<ServerWorkThread> server("127.0.0.1", 1024);
    
    return 0;
}