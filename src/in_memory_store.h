#pragma once
#include <string>
#include <unordered_map>

class InMemoryStore {
public:
    void set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string &value);
private:
    std::unordered_map<std::string, std::string> kvs_;
};
