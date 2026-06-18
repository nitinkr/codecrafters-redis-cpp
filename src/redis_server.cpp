#include <cerrno>
#include <cstdio>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include "redis_server.h"

void Server::start() {
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p; 
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int reuse = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family      = AF_INET;
    hints.ai_socktype    = SOCK_STREAM; 
    hints.ai_flags       = AI_PASSIVE;
    std::string port_str = std::to_string(port_);

    if ((rv = getaddrinfo(NULL, port_str.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        exit(1);
    }


    for(auto p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "server: socket" << std::endl;
            perror("server:socket");
            continue;
        }

        if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)) { 
            std::cerr << "setsockopt" << std::endl;
            exit(1);
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::cerr << sockfd << std::endl;
            continue;
        }
        std::cout << "found socket and done with bind() " << std::endl;
        break;
    }

    freeaddrinfo(servinfo);
    if(p == nullptr) {
        std::cerr << "server: filed to bind" << std::endl;
        exit(1);
    }

    if(listen(sockfd, BACKLOG) == -1) {
        std::cerr << "liten error " << std::endl;
        exit(1);
    }
   
    while(true) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            std::cerr << "accept new fd is -1" << std::endl; 
            std::cerr << strerror(errno) << std::endl; 
            continue;
        }
        std::cout << "client connected" << std::endl;
        char buff[1024];
        int byte_recved;
        while((byte_recved = recv(new_fd, buff, 1024, 0))  > 0) {
            std::string str = "+PONG\r\n";
            auto send_r  = send(new_fd, str.c_str(), str.length(), 0);
            if(send_r == -1) {
                std::cout << "error in send " << std::endl;
                std::cerr << strerror(errno) << std::endl;
            }
        }
    }
    close(sockfd);
}
