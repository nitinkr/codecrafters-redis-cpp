#include <cassert>
#include <cstdint>
#include <string>
#include <iostream>
#include "cmd_router.h"
#include "redis_types.h"
#include "resp_encoder.h"


std::string CmdRouter::process_cmd(Command &cmd) {
    if (auto it = cmds_.find(cmd.name); it != cmds_.end()) {  
        return it->second(cmd);
    }
    return RespEncoder::encode_error("cmd not found");
}

std::vector<std::string> CmdRouter::process(int txn_id, std::vector<Command>& commands) {
    std::vector<std::string> results;
    for(auto c : commands) {
        results.push_back(process_cmd(c));
    }
    return results;
}

int64_t CmdRouter::parse_timestamp(Command &cmd) {
    assert(cmd.name == "SET");
    if (cmd.args.size() < 4) {
        return -1;
    }
    int64_t ts;
    std::string arg = cmd.args[2].to_string();
    if (arg == "PX" || arg == "EX") {
        ts = std::stoll(cmd.args[3].to_string());
        if(arg == "EX") ts *= 1000;
    }
    return ts;
}

void CmdRouter::init_cmd_reg() {
        cmds_["PING"] = [this](Command&) -> std::string {
            return RespEncoder::pong_;
        };

        cmds_["ECHO"] = [this](Command &cmd) -> std::string {
            assert(cmd.args.size() == 1);
            return RespEncoder::encode_string(cmd.args[0].to_string());
        };

        cmds_["SET"] = [this](Command &cmd) -> std::string {
            if (cmd.args.size() < 2) {
                return RespEncoder::encode_error("Invalid command");
            }
            std::string key   = cmd.args[0].to_string();
            std::string value = cmd.args[1].to_string();
            std::cout << " in set lambda " << key << " " << value << std::endl;
            int64_t     ts = parse_timestamp(cmd);
            in_memory_store_.set(key, value, ts);
            return RespEncoder::simple_ok_;
        };

        cmds_["GET"] = [this](Command &cmd) -> std::string {
            assert(cmd.name == "GET");
            if (cmd.args.size() == 0) {
               return RespEncoder::encode_error("Invalid command" ); 
            }
            auto key = cmd.args[0].to_string();
            std::string value;
            if(in_memory_store_.get(key, value)) {
                return RespEncoder::encode_string(value);
            }
            return RespEncoder::null_bulk_str_;
        };

        cmds_["RPUSH"] = [this](Command &cmd) -> std::string {
            assert(cmd.name == "RPUSH");
            std::string key   = cmd.args[0].to_string();
            std::vector<std::string> args;
            for(int i=1; i<cmd.args.size(); i++) { args.push_back(cmd.args[i].to_string()); }
            auto len = in_memory_store_.append(key, args); 
            return RespEncoder::encode_int(len); 
        };

        cmds_["LRANGE"] = [this](Command &cmd) -> std::string {
            assert(cmd.name == "LRANGE");
            if(cmd.args.size() < 3) {
                return RespEncoder::encode_error("Invalid command");
            }
            std::string key = cmd.args[0].to_string();
            auto start = std::stoll(cmd.args[1].to_string());
            auto end   = std::stoll(cmd.args[2].to_string());
            auto res = in_memory_store_.lrang(key, start, end);
            return RespEncoder::encode_array(res);
        };

        cmds_["LPUSH"] = [this](Command &cmd) -> std::string {
            assert(cmd.name == "LPUSH");
            if (cmd.args.size() < 2) {
                return RespEncoder::encode_error("Invalid command");
            }
            std::string key = cmd.args[0].to_string();
            // TODO move to span
            std::vector<std::string> values;
            for(int i=1; i< cmd.args.size(); i++) { values.push_back(cmd.args[i].to_string()); }
            return RespEncoder::encode_int(in_memory_store_.prepend(key,values));
        };

        cmds_["LLEN"] = [this](Command &cmd) -> std::string {
            assert(cmd.name == "LLEN");
            if (cmd.args.size() != 1) {
                return RespEncoder::encode_error("Invalid command");
            }
            return RespEncoder::encode_int(in_memory_store_.list_length(cmd.args[0].to_string()));
        };
}
