#include "epoll_work_thread.h"

void *EpollWorkThread::StartThread(void *instance) {
    EpollWorkThread * work_thread = (EpollWorkThread *)instance;
    work_thread->Run();
    return work_thread;
}

void EpollWorkThread::Run() {
    std::cout<< "work thread:"<< thread_idx <<" is running!" << std::endl;
    
    epoll_fd_ = epoll_create(5);
    events_ = new std::vector<epoll_event>(64);
    AddFd(epoll_fd_, work_read_pipe_fd, (void *)NULL);
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
            ClientInfo * client_info = (ClientInfo *)(*events_)[i].data.ptr;

            if((*events_)[i].events & EPOLLIN) {
                if(client_info == NULL) {
                    FromMaster();
                }else {
                    OnRead(client_info);
                }
            }else if((*events_)[i].events & EPOLLOUT) {
                OnWrite(client_info);
            }else if((*events_)[i].events & (EPOLLERR | EPOLLHUP)){
                CloseClient(client_info);
            }
        }
    }
}

void EpollWorkThread::HandAcceptCompleted(ClientInfo * client_info) {

}

void EpollWorkThread::HandReadCompleted() {

}

void EpollWorkThread::OnWrite(ClientInfo * client_info) {

}

void EpollWorkThread::HandWrite(){

}

void EpollWorkThread::HandWriteCompleted(){

}


void EpollWorkThread::HandDisconnect(ClientInfo * client_info) {
    printf("thread:%d,client %d is close!\n", thread_idx, client_info->clinet_fd);
}


void EpollWorkThread::CloseClient(ClientInfo * client_info) {
    RemoveFd(epoll_fd_, client_info->clinet_fd);
    HandDisconnect(client_info);
}


void EpollWorkThread::OnRead(ClientInfo * client_info) {
    char buf[2048];
    int ret = Readn(client_info->clinet_fd, buf, 2048);
    if(ret == -1) {
        CloseClient(client_info);
        return;
    }
    buf[ret] = 0;
    printf(" thread_index: %d, recv = %s\n", thread_idx, buf);
}


int EpollWorkThread::Readn(int client_fd, char * buf, size_t size) {
    size_t have_read_size = 0;
    int ret = 0;
    while(have_read_size < size) {
        ret = read(client_fd, buf + have_read_size, size - have_read_size);
        if(ret <= 0) {
            if(ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return static_cast<int>(have_read_size);
            } else {
                return -1;
            }
        }
        have_read_size += ret;
    }
    return static_cast<int>(have_read_size);
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

            //TODO add a queue of ClinetInfo
            ClientInfo *client_info = new ClientInfo;
            client_info->clinet_fd = client_fd;
            socklen_t peer_len;
            getpeername(client_fd, (struct sockaddr *)&(client_info->client_address), &peer_len);

            AddFd(epoll_fd_, client_fd, (void *)client_info);
            HandAcceptCompleted(client_info);
            size = 0;
        }
    }
}