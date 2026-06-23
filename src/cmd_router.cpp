#include <cassert>
#include <cstdint>
#include <string>
#include <iostream>
#include "cmd_router.h"
#include "redis_types.h"
#include "resp_encoder.h"

void CmdRouter::process_cmd(Command &cmd) {
    if (auto it = cmds_.find(cmd.name_); it != cmds_.end()) {  
        it->second(cmd);
    } else {
        cmd.result_.push_back(RespEncoder::encode_error("cmd not found"));
    }
}

void CmdRouter::process(int txn_id, std::vector<Command>& commands) {
    std::vector<std::string> results;
    for(auto& c : commands) {
       process_cmd(c);
       if (c.state_ == c.READY) {
            c.state_ = Command::FINISHED;
       }
    }
}

int64_t CmdRouter::parse_timestamp(Command &cmd) {
    assert(cmd.name_ == "SET");
    if (cmd.args_.size() < 4) {
        return -1;
    }
    int64_t ts;
    std::string arg = cmd.args_[2].to_string();
    if (arg == "PX" || arg == "EX") {
        ts = std::stoll(cmd.args_[3].to_string());
        if(arg == "EX") ts *= 1000;
    }
    return ts;
}

void CmdRouter::init_cmd_reg() {
        cmds_["PING"] = [this](Command& cmd) -> void {
            cmd.result_.push_back(RespEncoder::pong_);
        };

        cmds_["ECHO"] = [this](Command &cmd) -> void {
            assert(cmd.args_.size() == 1);
            cmd.result_.push_back(RespEncoder::encode_string(cmd.args_[0].to_string()));
        };

        cmds_["SET"] = [this](Command &cmd) -> void {
            if (cmd.args_.size() < 2) {
                cmd.result_.push_back((RespEncoder::encode_error("Invalid command")));
                return;
            }
            std::string key   = cmd.args_[0].to_string();
            std::string value = cmd.args_[1].to_string();
            std::cout << " in set lambda " << key << " " << value << std::endl;
            int64_t     ts = parse_timestamp(cmd);
            in_memory_store_.set(key, value, ts);
            cmd.result_.push_back(RespEncoder::simple_ok_);
        };

        cmds_["GET"] = [this](Command &cmd) -> void {
            assert(cmd.name_ == "GET");
            if (cmd.args_.size() == 0) {
               cmd.result_.push_back((RespEncoder::encode_error("Invalid command")));
               return;
            }
            auto key = cmd.args_[0].to_string();
            std::string value;
            if(in_memory_store_.get(key, value)) {
                cmd.result_.push_back(RespEncoder::encode_string(value));
                return;
            }
            cmd.result_.push_back(RespEncoder::null_bulk_str_);
        };

        cmds_["RPUSH"] = [this](Command &cmd) -> void {
            assert(cmd.name_ == "RPUSH");
            std::string key   = cmd.args_[0].to_string();
            std::vector<std::string> args;
            for(int i=1; i<cmd.args_.size(); i++) { args.push_back(cmd.args_[i].to_string()); }
            auto len = in_memory_store_.list_append(key, args); 
            cmd.result_.push_back(RespEncoder::encode_int(len)); 
            if(args.size() > 0) cmd.state_ = Command::UNBLOCKING;
        };

        cmds_["LRANGE"] = [this](Command &cmd) -> void {
            assert(cmd.name_ == "LRANGE");
            if(cmd.args_.size() < 3) {
                cmd.result_.push_back(RespEncoder::encode_error("Invalid command"));
                return;
            }
            std::string key = cmd.args_[0].to_string();
            auto start = std::stoll(cmd.args_[1].to_string());
            auto end   = std::stoll(cmd.args_[2].to_string());
            auto res = in_memory_store_.list_range(key, start, end);
            cmd.result_.push_back(RespEncoder::encode_array(res));
        };

        cmds_["LPUSH"] = [this](Command &cmd) -> void {
            assert(cmd.name_ == "LPUSH");
            if (cmd.args_.size() < 2) {
                cmd.result_.push_back(RespEncoder::encode_error("Invalid command"));
                return;
            }
            std::string key = cmd.args_[0].to_string();
            // TODO move to span
            std::vector<std::string> values;
            for(int i=1; i< cmd.args_.size(); i++) { values.push_back(cmd.args_[i].to_string()); }
            cmd.result_.push_back(RespEncoder::encode_int(in_memory_store_.list_prepend(key,values)));
            if(values.size() > 0) cmd.state_ = Command::UNBLOCKING;
        };

        cmds_["LLEN"] = [this](Command &cmd) -> void {
            assert(cmd.name_ == "LLEN");
            if (cmd.args_.size() != 1) {
                cmd.result_.push_back(RespEncoder::encode_error("Invalid command"));
                return;
            }
            cmd.result_.push_back(RespEncoder::encode_int(in_memory_store_.list_length(cmd.args_[0].to_string())));
        };

        cmds_["LPOP"] = [this](Command &cmd) -> void {
            assert(cmd.name_ == "LPOP");
            if (cmd.args_.size() == 0) {
                cmd.result_.push_back(RespEncoder::encode_error("Inavlid command"));
                return;
            }
            auto count = (cmd.args_.size() < 2) ? 1 : std::stoll(cmd.args_[1].to_string());
            auto v = in_memory_store_.list_pop(cmd.args_[0].to_string(), count);
            if(v.size() == 1) cmd.result_.push_back(RespEncoder::encode_string(v[0]));
            else cmd.result_.push_back(v.size() ? RespEncoder::encode_array(v) : RespEncoder::null_bulk_str_);
        };
        cmds_["BLPOP"] = [this](Command &cmd) -> void {
            assert(cmd.name_ == "BLPOP");
            if (cmd.args_.size() == 0) {
                cmd.result_.push_back("Invalid command");
                return;
            }
            auto v = in_memory_store_.list_pop(cmd.args_[0].to_string(), 1);
            if (v.empty()) {
                cmd.state_ = Command::BLOCKED;
                std::cout << "[cmd_router] BLPOP was BLOCKED " << std::endl;
                return;
            }
            std::vector<std::string> res = {cmd.args_[0].to_string(), v[0]};
            cmd.result_.push_back(RespEncoder::encode_array(res));
            std::cout << "[cmd_router] BLPOP was not blocked " << std::endl;
        };
}
