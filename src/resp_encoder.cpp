#include <string>
#include "resp_encoder.h"

std::string RespEncoder::encode_array(std::vector<std::string>& arr) {
    std::string res = "*" + std::to_string(arr.size()) + "\r\n"; 
    for(auto& s : arr) res += encode_string(s);
    return res;
}

std::string RespEncoder::encode_string(const std::string &str) {
      return "$" +
             std::to_string(str.length()) +
             "\r\n" +
             str +
             "\r\n";
}
