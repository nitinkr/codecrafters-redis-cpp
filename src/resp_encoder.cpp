#include <string>
#include "resp_encoder.h"

std::string RespEncoder::encode_string(const std::string &str) {
      return "$" +
             std::to_string(str.length()) +
             "\r\n" +
             str +
             "\r\n";
}
