#ifndef EPOLL_SERVER_H
#define  EPOLL_SERVER_H

#include <sys/epoll.h>
#define BACKLOG  20
#define MAXEVENT 128
#define BUFFERSIZE 2048

class EpollServer {
public:
    EpollServer(int port) : port_(port), sockfd_(-1), epoll_fd_(-1) {};
    void start();
private:
    int  create_bind_listen();
    void handle_new_connection();
    bool is_error_event(uint32_t ev) { return (ev & (EPOLLHUP | EPOLLERR)); } 
    int port_;
    int sockfd_, epoll_fd_;
};

#endif
