#pragma once
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include "in_memory_store.h"
#include "redis_types.h"

class CmdRouter {
public:
    CmdRouter(InMemoryStore &ims) : in_memory_store_(ims) { init_cmd_reg(); }
    void process(int txn_id, std::vector<Command>& tokens);
    void process_cmd(Command &cmd);
private:
    void init_cmd_reg();
    int64_t parse_timestamp(Command &cmd);
    std::unordered_map<std::string, std::function<void(Command&)>> cmds_;
    InMemoryStore &in_memory_store_;
};
