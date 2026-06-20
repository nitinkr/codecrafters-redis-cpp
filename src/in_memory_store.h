#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class InMemoryStore {
public:
    void set(const std::string& key, const std::string& value, int64_t ts);
    bool get(const std::string& key, std::string &value);
    int append(const std::string& list, std::vector<std::string>& value);
    std::vector<std::string> lrang(const std::string& list, int start, int end);
    
private:
    int64_t current_time();
    bool is_expired(const std::string &key);
    void erase(const std::string &key);
    void insert_ts(const std::string &key, int64_t ts);
    std::unordered_map<std::string, std::string> kvs_;
    std::unordered_map<std::string, int64_t> timestamps_;
    std::unordered_map<std::string, std::vector<std::string>> lists_;
};
