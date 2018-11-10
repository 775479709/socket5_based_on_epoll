#include "epoll_work_thread.h"

void *EpollWorkThread::StartThread(void *instance) {
    EpollWorkThread * work_thread = (EpollWorkThread *)instance;
    work_thread->Run();
    return work_thread;
}

void EpollWorkThread::Run() {
    epoll_fd_ = epoll_create(5);
    events_ = new std::vector<epoll_event>(8);
    AddFd(epoll_fd_, work_read_pipe_fd);
    while(true){
        int event_num = epoll_wait(epoll_fd_, &(*events_->begin()),static_cast<int>(events_->size()), 1);
        if(event_num < 0 && errno != EINTR) {
            std::cout << "epoll failure" << std::endl;
            break;
        }
        if(event_num == events_->size()) {
            events_->resize(events_->size() * 2);
        }

        for(size_t i = 0; i < event_num; i++) {
            int sock_fd = (*events_)[i].data.fd;

            if(sock_fd == work_read_pipe_fd) {
                FromMaster();
            }else if((*events_)[i].events & EPOLLIN) {
                OnRead(sock_fd);
            }else if((*events_)[i].events & EPOLLOUT) {
                OnWrite(sock_fd);
            }else if((*events_)[i].events & (EPOLLERR | EPOLLHUP)){
                CloseClient(sock_fd);
            }
        }
    }
}

void EpollWorkThread::CloseClient(int client_fd) {
    
}


void EpollWorkThread::OnRead(int client_fd) {
    char buf[2048];
    int ret = Readn(client_fd, buf, 2048);
    buf[ret] = 0;
    printf(" thread_index: %d, recv = %s\n", thread_idx, buf);
}


int EpollWorkThread::Readn(int client_fd, char * buf, size_t size) {
    int have_read_size = 0;
    int ret = 0;
    while(true) {
        ret = read(client_fd, buf + have_read_size, size - have_read_size);
        if(ret <= 0) {
            if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return  static_cast<int>(have_read_size);
            } else {
                CloseClient(client_fd);
                return -1;
            }
        }
        have_read_size += ret;
    }
}

void EpollWorkThread::FromMaster() {
    char buf[4];
    int size = 0;
    int ret = 0;
    while(true) {
        ret = read(work_read_pipe_fd, buf + size, 4 - size);
        if((ret == -1 && errno == EAGAIN) || ret == 0) {
            break;
        }
        size += ret;
        if(size == 4) {
            int client_fd;
            memcpy(&client_fd, buf, 4);
            SetNonblocking(client_fd);
            AddFd(epoll_fd_, client_fd);
            HandAcceptCompleted(client_fd);
            size = 0;
        }
    }
}