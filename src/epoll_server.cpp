#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "epoll_server.h"
#include "resp_parser.h"
#include "cmd_router.h"

void EpollServer::del_connection(Connection *conn) {
    if(conn != nullptr) {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, conn->fd_, nullptr);
        close(conn->fd_);
        delete conn;
    }
}

void EpollServer::handle_write(Connection *conn) {
    int byte_sent = send(conn->fd_, conn->send_buff_ + conn->send_idx_, conn->send_len_ - conn->send_idx_, 0);
    if(byte_sent == -1) {
        return;
    }
    conn->send_idx_ += byte_sent;

    if(conn->send_idx_ == conn->send_len_) {
        conn->send_idx_ = conn->send_len_ = 0; 
        struct epoll_event ev;
        ev.data.ptr = conn;
        ev.events   = EPOLLIN;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, conn->fd_, &ev);
    }
}


int EpollServer::handle_read(Connection  *conn) {
    int bytes_received = recv(conn->fd_, conn->recv_buff_ + conn->recv_idx_, BUFFERSIZE - conn->recv_idx_, 0);        
    std::cout << "bytes_revceived " << bytes_received << std::endl;
    if(bytes_received <= 0) {
        if(bytes_received == 0) {
            std::cout << "closing connection " << std::endl;
            del_connection(conn);
        }
        return bytes_received;
    }
    for(int i = conn->recv_idx_; i < conn->recv_idx_ + bytes_received; i++) {
        std::cout << conn->recv_buff_[i];
    }
    std::cout << std::endl;
        
    conn->recv_idx_ = bytes_received;
    struct epoll_event ev;
    ev.data.ptr = conn;
    ev.events   = EPOLLOUT;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, conn->fd_, &ev);
    return bytes_received;
}

int EpollServer::create_bind_listen() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int reuse = 1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype =SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    std::string port_str = std::to_string(port_);

    if(int rv = getaddrinfo(NULL, port_str.c_str(), &hints, &servinfo); rv != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }

    for(p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "server:: socket" << std::endl;
            perror("server::socket");
            continue;
        }

        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)) { 
            perror("setsockopt ");
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    if (p == nullptr) {
        perror("server: failed to bind"); 
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("sigaction");
        exit(1);
    }
    return sockfd;
}

void EpollServer::handle_new_connection() {
    struct sockaddr_storage client_addr;
    socklen_t sin_size = sizeof(client_addr);

    int client_fd = accept(sockfd_, (struct sockaddr *)&client_addr, &sin_size);
    if(client_fd == -1) {
        perror("server:accept");
        return;
    }
    epoll_event ev;
    Connection *conn = new Connection(client_fd); 
    ev.data.ptr = conn;
    ev.events  = EPOLLIN;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev); 
}

void EpollServer::start() {
    sockfd_ = create_bind_listen();
    int new_fd, num_fds, bytes_received;
    socklen_t sin_size;
    char send_buff[1024], recv_buff[1024];
    Connection *conn = nullptr;
    struct sockaddr_storage client_addr;
    struct epoll_event epoll_ev, events[MAXEVENT];
    char s[INET6_ADDRSTRLEN];

    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    epoll_ev.events    = EPOLLIN;
    conn = new Connection(sockfd_);
    epoll_ev.data.ptr = conn;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sockfd_, &epoll_ev); 
    int t = 0;
    while(true) {
        std::cout << "waiting t= " << (t++) << std::endl;
        num_fds = epoll_wait(epoll_fd_, events, MAXEVENT, -1);
        for(int i=0; i < num_fds; i++) {
            conn   = (Connection *) events[i].data.ptr;
            new_fd = conn->fd_;
            std::cout << "procssing event for fd: " << new_fd << std::endl;
            if(is_error_event(events[i].events)) {
                std::cout << "found error event " << new_fd << std::endl;
                del_connection(conn);
                if(new_fd == sockfd_) {
                    perror("error on server socket");
                    close(epoll_fd_);
                    exit(1);
                }
                continue;
            }
            
            if(new_fd == sockfd_) {
                std::cout << "handling new connection on fd " << new_fd << std::endl;
                handle_new_connection();
            } else if(events[i].events & EPOLLIN) {
                std::cout << "procesing read on clint fd " << new_fd << std::endl;
                bytes_received = handle_read(conn);
                std::cout << "done with read " << std::endl;
                if(bytes_received > 0) process_command(conn);
                std::cout << "done with process_command " << std::endl;
            } else if(events[i].events & EPOLLOUT) {
                std::cout << "handling write event for client fd " << new_fd << std::endl;
                handle_write(conn);
            }
        }
    }
    close(sockfd_);
    close(epoll_fd_);
}

bool EpollServer::process_command(Connection *conn) {
    RespParser r(conn);
    r.parse();
    if(!r.IsValid()) {
        // the bytes are not valid command or command sequence
        // we wait for more bytes to arrive
        epoll_event ev;
        ev.data.ptr = conn;
        ev.events   = EPOLLIN;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, conn->fd_, &ev);
        return false;
    }
    auto results = router_.process(conn->fd_, r.get_values());
    for(auto& str : results) {
        memcpy(conn->send_buff_ + conn->send_len_, str.c_str(), str.length()); 
        conn->send_len_ += str.length();
    }
    conn->recv_idx_ = 0;
    return true;
}
