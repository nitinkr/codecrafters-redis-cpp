#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>

class InMemoryStore {
public:
    void set(const std::string& key, const std::string& value, int64_t ts);
    bool get(const std::string& key, std::string &value);
    int list_append(const std::string& list, std::vector<std::string>& value);
    int list_prepend(const std::string& list, std::vector<std::string>& values);
    std::vector<std::string> list_pop(const std::string& list, int count);
    int list_length(const std::string& list);
    std::vector<std::string> list_range(const std::string& list, int start, int end);
    int64_t current_time();
    
private:
    bool is_expired(const std::string &key);
    void erase(const std::string &key);
    void insert_ts(const std::string &key, int64_t ts);
    std::unordered_map<std::string, std::string> kvs_;
    std::unordered_map<std::string, int64_t> timestamps_;
    std::unordered_map<std::string, std::list<std::string>> lists_;
};
