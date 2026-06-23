#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>
#include <iostream>
#include <list>
#include "in_memory_store.h"

void InMemoryStore::set(const std::string& key, const std::string& value, int64_t ts) {
    kvs_[key] = value;
    insert_ts(key, ts);
}

void InMemoryStore::insert_ts(const std::string &key, int64_t ts) {
    if(ts > 0) {
        timestamps_[key] = current_time() + ts; 
    }
}

int64_t InMemoryStore::current_time() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<
                    std::chrono::milliseconds>(duration).count();
}

bool InMemoryStore::is_expired(const std::string &key) {
    if (auto it = timestamps_.find(key); it != timestamps_.end()) {
        auto ts = current_time();
        return ts >= it->second;
    }
    return false;
}

void InMemoryStore::erase(const std::string& key) {
    kvs_.erase(key);
    timestamps_.erase(key);
}

bool InMemoryStore::get(const std::string& key, std::string &value) {
    if (auto it = kvs_.find(key); it != kvs_.end()) {
        if (!is_expired(key)) {
            value = it->second;
            return true;
        }
        erase(key);
    }
    return false;
}

std::vector<std::string> InMemoryStore::list_range(const std::string& list, int start, int end) {
    std::vector<std::string> result;
    std::cout << "start " << start << "end " << end << std::endl;
    if (auto it = lists_.find(list); it != lists_.end()) {
        auto& v = it->second;
        int size = v.size();
        start = (start < 0) ? std::max(0, size + start) : start;
        end   = (end   < 0) ? std::max(0, size + end )  : end;
        std::cout << "start " << start << "end " << end << " v.size() " << v.size() << std::endl;
        auto l_it = std::next(v.begin(), start);
        for(int i=start; i<=end && i < v.size(); i++, l_it++) {
            result.push_back(*l_it);
        }
    }
    return result;
}

int InMemoryStore::list_length(const std::string& list) {
    if (auto it = lists_.find(list); it != lists_.end()) {
        return it->second.size();
    }
    return 0;
}

int InMemoryStore::list_prepend(const std::string& list, std::vector<std::string>& values) {
    if(values.size() == 0) return 0;
    auto& v = lists_[list];
    for(auto& val : values) {
        v.push_front(val);
    }
    return v.size();
}

int InMemoryStore::list_append(const std::string& list, std::vector<std::string>& values) {
    auto& v = lists_[list];
    for(auto& value : values) {
        v.push_back(value);
    }
    return v.size();
}

std::vector<std::string> InMemoryStore::list_pop(const std::string& list, int count) {
    std::vector<std::string> result;
    if (auto it = lists_.find(list); it != lists_.end()) {
        auto& l = it->second;
        while(!l.empty() && count) {
            result.push_back(l.front());
            l.pop_front();
            count--;
        }
    }
    return result;
}

