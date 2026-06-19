#include <cstdint>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "epoll_server.h"
#include "resp_parser.h"

#define PORT 6379
/*
void test(std::string& str) {
    std::cout << "*****parsing str ******** " << str << std::endl;
    Connection c(-1);
    memcpy(c.recv_buff_, str.c_str(), str.length()); 
    c.recv_idx_ = str.length();
    RespParser p(&c);
    std::cout << "parse " << p.parse() << std::endl;
    auto values = p.get_values();
    std::cout << "total values " << values.size() << std::endl;
    for (auto& v : p.get_values()) {
        if(v.type_ == RespValue::INTEGER64) std::cout <<  std::get<int64_t>(v.value_) << std::endl;
        else std::cout <<  std::get<std::string>(v.value_) << std::endl;
    }
}

void test_main() {
    std::string strs[] = {
        "*3\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$3\r\nbar\r\n",
        "*2\r\n$3\r\nGET\r\n$3\r\nfoo\r\n",
        "*1\r\n$4\r\nPING\r\n",
        "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nAlice\r\n",
        "*0\r\n",
        "*2\r\n$4\r\nLPOP\r\n$0\r\n\r\n",
        "*3\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$0\r\n\r\n",
        "*3\r\n$3\r\nSET\r\n$10\r\nstrawberry\r\n$6\r\nbanana\r\n",
    };

    for(auto& s : strs) {
        test(s);
    }
}

void test_simple_str() {
    Connection c(-1);
    std::string str = "+OK\r\n";
    memcpy(c.recv_buff_, str.c_str(), str.length()); 
    c.recv_idx_ = str.length();
    RespParser p(&c);
    std::cout << "parse " << p.parse() << std::endl;
    auto values = p.get_values();
    std::cout << "total values " << values.size() << std::endl;
    for (auto& v : p.get_values()) {
        if(v.type_ == RespValue::INTEGER64) std::cout <<  std::get<int64_t>(v.value_) << std::endl;
        else std::cout <<  std::get<std::string>(v.value_) << std::endl;
    }
}

void test_bulk_str() {
    Connection c(-1);
    std::string str = "$4\r\nPING\r\n";
    memcpy(c.recv_buff_, str.c_str(), str.length()); 
    c.recv_idx_ = str.length();
    RespParser p(&c);
    std::cout << "parse " << p.parse() << std::endl;
    auto values = p.get_values();
    std::cout << "total values " << values.size() << std::endl;
    for (auto& v : p.get_values()) {
        if(v.type_ == RespValue::INTEGER64) std::cout <<  std::get<int64_t>(v.value_) << std::endl;
        else std::cout <<  std::get<std::string>(v.value_) << std::endl;
    }
}




void test_set() {
    Connection c(-1);
    std::string str = "*3\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$3\r\nbar\r\n";
    memcpy(c.recv_buff_, str.c_str(), str.length()); 
    c.recv_idx_ = str.length();
    RespParser p(&c);
    std::cout << "parse " << p.parse() << std::endl;
    auto values = p.get_values();
    std::cout << "total values " << values.size() << std::endl;
    for (auto& v : p.get_values()) {
        if(v.type_ == RespValue::INTEGER64) std::cout <<  std::get<int64_t>(v.value_) << std::endl;
        else std::cout <<  std::get<std::string>(v.value_) << std::endl;
    }
}
*/
int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  std::cout << "argc " << argc << std::endl;
  for(int i=0; i<argc; i++) {
      std::cout << "i: " << i << " " << argv[i] << std::endl;
  }
  if (argc >= 2 && strcmp(argv[1], "test") == 0) {
    //test_main();
    return 0;
  }
  EpollServer s(PORT);
  s.start();
  return 0;
}
