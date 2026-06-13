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


int EpollServer::create_bind_listen() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int reuse = 1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
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
    ev.data.fd = client_fd;
    ev.events  = EPOLLIN;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev); 
}

void EpollServer::start() {
    sockfd_ = create_bind_listen();
    int new_fd, num_fds;
    socklen_t sin_size;
    char send_buff[1024], recv_buff[1024];
    int byte_received;
    struct sockaddr_storage client_addr;
    struct epoll_event epoll_ev, events[MAXEVENT];
    char s[INET6_ADDRSTRLEN];

    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    epoll_ev.events    = EPOLLIN;
    epoll_ev.data.fd   = sockfd_; 

    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sockfd_, &epoll_ev); 

    while(true) {
        num_fds = epoll_wait(epoll_fd_, events, MAXEVENT, -1);
 
        for(int i=0; i < num_fds; i++) {
            new_fd = events[i].data.fd;
            if(is_error_event(events[i].events)) {
                if(new_fd == sockfd_) {
                    perror("error on server socket");
                    close(sockfd_);
                    close(epoll_fd_);
                    exit(1);
                }
                epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, new_fd, nullptr);
                close(new_fd);
                continue;
            }
            
            if(new_fd == sockfd_) {
                handle_new_connection();
            } else if(events[i].events & EPOLLIN) {
                byte_received = recv(new_fd, recv_buff, BUFFERSIZE, 0);
                if(byte_received <= 0) {
                    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, new_fd, nullptr);
                    close(new_fd);
                    continue;
                }
                epoll_ev.events    = EPOLLOUT;
                epoll_ev.data.fd   = new_fd;
                epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, new_fd, &epoll_ev);
            } else if(events[i].events & EPOLLOUT) {
                // handle write();
                std::string pong = "+PONG\r\n";
                send(new_fd, pong.c_str(), pong.length(), 0);
                epoll_ev.events  = EPOLLIN;
                epoll_ev.data.fd = new_fd; 
                epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, new_fd, &epoll_ev);
            }
        }
    }
    close(sockfd_);
    close(epoll_fd_);
}
