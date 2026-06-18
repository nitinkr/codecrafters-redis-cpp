#include <string>
#include "in_memory_store.h"

void InMemoryStore::set(const std::string& key, const std::string& value) {
   kvs_[key] = value; 
}

bool InMemoryStore::get(const std::string& key, std::string &value) {
    if (auto it = kvs_.find(key); it != kvs_.end()) {
        value = it->second;
        return true;
    }
    return false;
}

