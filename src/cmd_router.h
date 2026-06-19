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
    std::vector<std::string> process(int txn_id, std::vector<RespValue>& tokens);
private:
    void init_cmd_reg();
    int64_t parse_timestamp(std::vector<RespValue> &tokens, int &id);
    std::unordered_map<std::string, std::function<std::string(std::vector<RespValue>&, int&)>> cmds_;
    InMemoryStore &in_memory_store_;
};
