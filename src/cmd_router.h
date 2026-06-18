#pragma once
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
    std::unordered_map<std::string, std::function<std::string(std::vector<RespValue>&, int &id)>> cmds_;
    InMemoryStore &in_memory_store_;
};
