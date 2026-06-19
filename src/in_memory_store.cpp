#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>
#include <iostream>
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
        std::cout << "in is_expired cur ts" << ts << " -- old ts " << it->second << std::endl;  
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

