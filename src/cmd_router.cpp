#include "cmd_router.h"
#include "resp_encoder.h"


void CmdRouter::process(int txn_id, std::vector<RespValue>& tokens) {
    int start = 0;
    std::string str;
    while(start < tokens.size()) {
        str = tokens[start++].to_string();
        if(auto it = cmds_.find(str); it != cmds_.end()) {
            it->second(tokens, start);
        } else {
            results.push_back(RespEncoder::encode_error("cmd not found"));
        }
    }
}
void CmdRouter::init_cmd_reg() {
        cmds_["PING"] = [this](std::vector<RespValue>& tokens, int &id) {
            results.push_back(RespEncoder::pong_);
        };

        cmds_["ECHO"] = [this](std::vector<RespValue>& tokens, int &id) {
            results.push_back(RespEncoder::encode_string(tokens[id++].to_string()));
        };
}
