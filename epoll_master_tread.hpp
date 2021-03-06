#ifndef EPOLLMASTERTHREAD_HPP
#define EPOLLMASTERTHREAD_HPP
#include "epoll_common.hpp"

template <class WorkThread>
class EpollMasterThread {
private:
    class WorkThreadInfo {
    public:
        size_t client_num;
        int master_read_pipe_fd;
        int master_write_pipe_fd;
        WorkThread* epoll_work_thread;
        WorkThreadInfo()
            : client_num(0)
            , epoll_work_thread(nullptr)
        {
        }
    };

public:
    EpollMasterThread(const char* ip, int port, size_t work_thread_num = 2);
    ~EpollMasterThread();

private:
    void Run();

private:
    int work_thread_num_;
    int epoll_fd_;
    int listen_fd_;
    WorkThreadInfo* work_thread_info_;
    struct sockaddr_in local_address_;
    std::vector<epoll_event>* events_;
    pthread_t* threads_;
};

template <class WorkThread>
EpollMasterThread<WorkThread>::EpollMasterThread(const char* ip, int port, size_t work_thread_num)
{
    work_thread_num_ = work_thread_num;
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    assert(listen_fd_ > 0);
    bzero(&local_address_, sizeof(local_address_));
    struct linger tmp = { 1, 0 };
    //TODO:check argv:SO_REUSEADDR
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));

    int ret = 0;
    local_address_.sin_family = AF_INET;
    if (ip == nullptr) {
        local_address_.sin_addr.s_addr = 0;
    } else {
        inet_pton(AF_INET, ip, &local_address_.sin_addr);
    }
    local_address_.sin_port = htons(port);

    ret = bind(listen_fd_, (struct sockaddr*)&local_address_, sizeof(local_address_));
    if (ret < 0) {
        std::cout << "master thread :bind error!" << std::endl;
    }
    ret = listen(listen_fd_, 50);
    if (ret < 0) {
        std::cout << "master thread : listen error!" << std::endl;
    }
    epoll_fd_ = epoll_create(5);
    AddFd(epoll_fd_, listen_fd_, (void*)NULL);

    work_thread_info_ = new WorkThreadInfo[work_thread_num];
    threads_ = new pthread_t[work_thread_num];
    for (size_t i = 0; i < work_thread_num; i++) {
        work_thread_info_[i].epoll_work_thread = new WorkThread();
        int fds1[2], fds2[2];
        if (pipe(fds1) == -1 || pipe(fds2) == -1) {
            std::cout << "pipe error" << std::endl;
        }
        SetNonblocking(fds1[0]);
        SetNonblocking(fds1[1]);
        SetNonblocking(fds2[0]);
        SetNonblocking(fds2[1]);
        AddFd(epoll_fd_, fds1[0], (void*)&work_thread_info_[i]);
        work_thread_info_[i].master_read_pipe_fd = fds1[0];
        work_thread_info_[i].master_write_pipe_fd = fds2[1];
        work_thread_info_[i].epoll_work_thread->work_read_pipe_fd = fds2[0];
        work_thread_info_[i].epoll_work_thread->work_write_pipe_fd = fds1[1];
        work_thread_info_[i].epoll_work_thread->thread_idx = i;
        if (pthread_create(threads_ + i, NULL, WorkThread::StartThread, work_thread_info_[i].epoll_work_thread) != 0) {
            std::cout << "create thread error!" << std::endl;
            throw std::exception();
        }
        if (pthread_detach(threads_[i])) {
            std::cout << "detach thread error!" << std::endl;
            throw std::exception();
        }
    }
    Run();
}

template <class WorkThread>
EpollMasterThread<WorkThread>::~EpollMasterThread()
{
    for (int i = 0; i < work_thread_num_; i++) {
        int data = 0;
        write(work_thread_info_[i].master_write_pipe_fd, &data, 4);
        while (true) {
            sleep(1);
            if (work_thread_info_[i].epoll_work_thread->is_stop) {
                delete work_thread_info_[i].epoll_work_thread;
                close(work_thread_info_[i].master_read_pipe_fd);
                close(work_thread_info_[i].master_write_pipe_fd);
                break;
            }
        }
    }
    sleep(1);
    close(epoll_fd_);
    close(listen_fd_);
    delete events_;
    delete work_thread_info_;
    delete threads_;

    //TODO: Check sub thread whether exit.
}

template <class WorkThread>
void EpollMasterThread<WorkThread>::Run()
{
    std::cout << "master thread is running!" << std::endl;
    events_ = new std::vector<epoll_event>(8);
    size_t current_thread_index = 0;
    while (true) {
        int event_num = epoll_wait(epoll_fd_, &(*events_->begin()), static_cast<int>(events_->size()), 1);
        if (event_num < 0 && errno != EINTR) {
            std::cout << "epoll failure" << std::endl;
            break;
        }
        if (event_num == events_->size()) {
            events_->resize(events_->size() * 2);
            std::cout << "master events resize :" << events_->size() << std::endl;
        }

        for (size_t i = 0; i < event_num; i++) {
            WorkThreadInfo* work_thread_info = (WorkThreadInfo*)(*events_)[i].data.ptr;

            if ((*events_)[i].events & EPOLLIN) {
                if (work_thread_info == NULL) {
                    int client_fd;
                    while ((client_fd = accept(listen_fd_, (struct sockaddr*)NULL, NULL)) > 0) {
                        int ret = write(work_thread_info_[current_thread_index].master_write_pipe_fd, &client_fd, sizeof(client_fd));
                        if (ret != sizeof(client_fd)) {
                            printf("master write pipe error!");
                            throw std::exception();
                        }
                        current_thread_index++;
                        if (current_thread_index >= work_thread_num_) {
                            current_thread_index = 0;
                        }
                    }

                } else {
                }
            } else {
            }
        }
    }
}

#endif