#pragma once
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>
#include "redis_types.h"

class RespParser {
public:
    RespParser(Connection *conn) : buff_(conn->recv_buff_), length_(conn->recv_idx_),
                                   current_(0), is_valid_(true) {};
    bool IsValid() { return is_valid_; }
    bool parse_all();
    bool parse();
    std::vector<Command>& get_cmds() { return cmds_; }
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
    std::vector<Command> cmds_;
};
