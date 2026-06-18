#pragma once 
#include <string>
#include <variant>
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

struct RespValue {
    enum Type {
        STRING,
        NUMBER,
        NULL_STRING,
        INTEGER64,
        DOUBLE,
        BIGNUMBER,
    };
    std::variant<std::string, int64_t, double> value_;
    Type type_;
    std::string to_string() {
        return  std::visit(
              [](auto &arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return arg;
            } else {
                return std::to_string(arg);
            }
        }, value_);
    }
};
