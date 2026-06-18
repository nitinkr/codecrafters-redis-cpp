#ifndef EPOLL_SERVER_H
#define  EPOLL_SERVER_H

#include <sys/epoll.h>
#include "in_memory_store.h"
#include "cmd_router.h"
#include "redis_types.h"


class EpollServer {
public:
    EpollServer(int port) : port_(port), sockfd_(-1), epoll_fd_(-1), router_(in_memory_store_) {}
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
    InMemoryStore in_memory_store_;
    CmdRouter router_;
};

#endif
