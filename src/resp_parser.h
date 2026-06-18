#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>
#include "epoll_server.h"

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

class RespParser {
public:
    RespParser(Connection *conn) : buff_(conn->recv_buff_), length_(conn->recv_idx_),
                                   current_(0), is_valid_(true) {};
    bool IsValid() { return is_valid_; }
    bool parse();
    std::vector<RespValue>& get_values() { return values_; }
private:
    void parse_string();
    void parse_simple_string();
    void parse_bulk_string();
    void parse_int64();
    void parse_array();
    char peek();
    bool match(char ch);
    char advance();
    bool is_end() { return current_ >= length_; }
    bool is_crlf();
    int  parse_length();
    const char *buff_;
    int current_;
    const int length_;
    bool  is_valid_;
    std::vector<RespValue> values_;
};
