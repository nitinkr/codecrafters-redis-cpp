#ifndef EPOLL_SERVER_H
#define  EPOLL_SERVER_H

#include <sys/epoll.h>
#define BACKLOG  20
#define MAXEVENT 128
#define BUFFERSIZE 4096 

struct Connection {
    Connection(int fd): fd_(fd) {} 
    int  fd_;
    int  recv_idx_   = 0;
    int  send_idx_ = 0; 
    int  send_len_ = 0;
    char recv_buff_[BUFFERSIZE] = {0};
    char send_buff_[BUFFERSIZE] = {0};
};

class EpollServer {
public:
    EpollServer(int port) : port_(port), sockfd_(-1), epoll_fd_(-1) {};
    void start();
private:
    int  create_bind_listen();
    void del_connection(Connection *conn);
    bool process_command(Connection *conn);
    void handle_new_connection();
    int handle_read(Connection  *conn);
    void handle_write(Connection *conn);
    bool is_error_event(uint32_t ev) { return (ev & (EPOLLHUP | EPOLLERR)); } 
    int port_;
    int sockfd_, epoll_fd_;
};

#endif
