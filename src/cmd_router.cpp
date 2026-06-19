#include <cstdint>
#include <string>
#include <iostream>
#include "cmd_router.h"
#include "resp_encoder.h"

std::vector<std::string> CmdRouter::process(int txn_id, std::vector<RespValue>& tokens) {
    int start = 0;
    std::string str;
    std::vector<std::string> results;
    while(start < tokens.size()) {
        str = tokens[start++].to_string();
        if(auto it = cmds_.find(str); it != cmds_.end()) {
            results.push_back(it->second(tokens, start));
        } else {
            results.push_back(RespEncoder::encode_error("cmd not found"));
        }
    }
    return results;
}

int64_t CmdRouter::parse_timestamp(std::vector<RespValue> &tokens, int &id) {
    if (id == tokens.size()) {
        return -1;
    }
    int64_t ts;
    std::string arg = tokens[id].to_string();
    if (arg == "PX" || arg == "EX") {
       id++;
       if(id < tokens.size()) {
           ts = std::stoll(tokens[id++].to_string());
           if(arg == "EX") ts *= 1000;
        }
    }
    return ts;
}

void CmdRouter::init_cmd_reg() {
        cmds_["PING"] = [this](std::vector<RespValue>& tokens, int &id) -> std::string {
            return RespEncoder::pong_;
        };

        cmds_["ECHO"] = [this](std::vector<RespValue>& tokens, int &id) -> std::string {
            return RespEncoder::encode_string(tokens[id++].to_string());
        };

        cmds_["SET"] = [this](std::vector<RespValue>& tokens, int &id) -> std::string {
            if (id + 1 >= tokens.size()) {
                return RespEncoder::encode_error("Invalid command");
            }
            std::string key   = tokens[id++].to_string();
            std::string value = tokens[id++].to_string();
            std::cout << " in set lambda " << key << " " << value << std::endl;
            int64_t     ts = parse_timestamp(tokens, id);
            in_memory_store_.set(key, value, ts);
            return RespEncoder::simple_ok_;
        };

        cmds_["GET"] = [this](std::vector<RespValue>& tokens, int &id) -> std::string {
            if (id >= tokens.size()) {
               return RespEncoder::encode_error("Invalid command" ); 
            }
            auto key = tokens[id++].to_string(); 
            std::string value;
            if(in_memory_store_.get(key, value)) {
                return RespEncoder::encode_string(value);
            }
            return RespEncoder::null_bulk_str_;
        };
}
