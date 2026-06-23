#pragma once
#include <cstdint>
#include<string>
#include <vector>

class RespEncoder {
public:
    constexpr static char simple_ok_[]     = "+OK\r\n";
    constexpr static char pong_[]          = "+PONG\r\n";
    constexpr static char null_bulk_str_[] = "$-1\r\n";
    constexpr static char null_bulk_array_[] = "*-1\r\n";
    static std::string encode_int(int64_t i) { return ":" + std::to_string(i) + "\r\n"; }
    static std::string encode_string(const std::string &str);
    static std::string encode_error(const std::string  &str) { return str;}
    static std::string encode_array(const std::vector<std::string>& arr);
};
