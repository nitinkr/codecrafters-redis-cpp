#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include "resp_parser.h"

class CmdRouter {
public:
    CmdRouter() {
        init_cmd_reg();
    }
    void process(int txn_id, std::vector<RespValue>& tokens);
    std::vector<std::string>& get_results() { return results; }
private:
    void init_cmd_reg();
    std::vector<std::string> results;
    std::unordered_map<std::string, std::function<void(std::vector<RespValue>&, int &id)>> cmds_;
};
