#ifndef EPOLLWORKTHREAD_HPP
#define EPOLLWORKTHREAD_HPP
#include "epoll_common.hpp"
#include "memory_pool.hpp"

class EpollWorkThread
{
  public:
    struct Client
    {
        struct sockaddr_in client_address;
        int clinet_fd;
        void *data = nullptr;
    };

  public:
    EpollWorkThread() : is_stop_(false), events_(nullptr), client_pool_(nullptr) {}
    ~EpollWorkThread();
    static void *StartThread(void *instance);
    void Run();
    void CloseClient(Client *client);


    virtual void HandAcceptCompleted(Client *client) = 0;
    virtual void HandReadEvent(Client *client) = 0;
    virtual void HandWriteEvent(Client *client) = 0;
    virtual void HandDisconnect(Client *client) = 0;

  public:
    int work_read_pipe_fd;
    int work_write_pipe_fd;
    int thread_idx;
    int epoll_fd;

  private:
    void FromMaster();

  private:
    bool is_stop_;
    std::vector<epoll_event> *events_;
    MemoryPool<Client, 1024> *client_pool_;
};

EpollWorkThread::~EpollWorkThread() {
    delete events_;
    delete client_pool_;
    close(epoll_fd);
    close(work_read_pipe_fd);
    close(work_write_pipe_fd);
}

void *EpollWorkThread::StartThread(void *instance)
{
    EpollWorkThread *work_thread = (EpollWorkThread *)instance;
    work_thread->Run();
    return work_thread;
}

void EpollWorkThread::Run()
{
    std::cout << "work thread:" << thread_idx << " is running!" << std::endl;
    client_pool_ = new MemoryPool<Client, 1024>();

    epoll_fd = epoll_create(5);
    events_ = new std::vector<epoll_event>(64);
    AddFd(epoll_fd, work_read_pipe_fd, (void *)NULL);
    while (true)
    {
        int event_num = epoll_wait(epoll_fd, &(*events_->begin()), static_cast<int>(events_->size()), 1);
        if (event_num < 0 && errno != EINTR)
        {
            std::cout << "epoll failure" << std::endl;
            break;
        }
        if (event_num == events_->size())
        {
            events_->resize(events_->size() * 2);
        }

        for (size_t i = 0; i < event_num; i++)
        {
            Client *client = (Client *)(*events_)[i].data.ptr;
            if ((*events_)[i].events & EPOLLHUP)
            {
                CloseClient(client);
                continue;
            }
            if ((*events_)[i].events & EPOLLOUT)
            {
                HandWriteEvent(client);
            }
            if ((*events_)[i].events & EPOLLIN)
            {
                if (client == NULL)
                {
                    FromMaster();
                }
                else
                {
                    HandReadEvent(client);
                }
            }
            
        }
    }
}

void EpollWorkThread::CloseClient(Client *client)
{
    HandDisconnect(client);
    RemoveFd(epoll_fd, client->clinet_fd);
    close(client->clinet_fd);
    client_pool_->Delete(client);
}

void EpollWorkThread::FromMaster()
{
    char buf[4];
    int size = 0;
    int ret = 0;
    while (true)
    {
        ret = read(work_read_pipe_fd, buf + size, 4 - size);
        if ((ret == -1 && errno == EAGAIN) || ret == 0)
        {
            break;
        }
        size += ret;
        if (size == 4)
        {
            int client_fd;
            memcpy(&client_fd, buf, 4);
            SetNonblocking(client_fd);

            Client *client = client_pool_->New();
            client->clinet_fd = client_fd;
            socklen_t peer_len;
            getpeername(client_fd, (struct sockaddr *)&(client->client_address), &peer_len);

            AddFd(epoll_fd, client_fd, (void *)client);
            HandAcceptCompleted(client);
            size = 0;
        }
    }
}

// //TODO:Readn while size == have_read_size need read again
// int EpollWorkThread::Readn(int client_fd, char *buf, size_t size)
// {
//     size_t have_read_size = 0;
//     int ret = 0;
//     while (have_read_size < size)
//     {
//         ret = read(client_fd, buf + have_read_size, size - have_read_size);
//         if (ret <= 0)
//         {
//             if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
//             {
//                 return static_cast<int>(have_read_size);
//             }
//             else
//             {
//                 return -1;
//             }
//         }
//         have_read_size += ret;
//     }
//     return static_cast<int>(have_read_size);
// }

// int EpollWorkThread::Writen(int clinet_fd, char *buf, size_t size)
// {
//     size_t have_write_size = 0;
//     int ret = 0;
//     while (have_write_size < size)
//     {
//         ret = write(clinet_fd, buf + have_write_size, size - have_write_size);
//         if (ret <= 0)
//         {
//             if (ret == -1 && errno == EAGAIN)
//             {
//                 return static_cast<int>(have_write_size);
//             }
//             else
//             {
//                 return -1;
//             }
//         }
//         have_write_size += ret;
//     }
//     return static_cast<int>(have_write_size);
// }



#endif