#include "SeenURLStore.h"

SeenURLStore::SeenURLStore() {
    // Constructor initializes the internal HashMap automatically
}

void SeenURLStore::markSeen(const std::string& url) {
    std::lock_guard<std::mutex> lock(mtx);
    map.put(url, true);
}

bool SeenURLStore::isSeen(const std::string& url) const {
    std::lock_guard<std::mutex> lock(mtx);
    return map.contains(url);
}

int SeenURLStore::size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return map.size();
}
