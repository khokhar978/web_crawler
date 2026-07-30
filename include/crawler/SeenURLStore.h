#ifndef SEEN_URL_STORE_H
#define SEEN_URL_STORE_H

#include <string>
#include <mutex>
#include "HashMap.h"

// Tracks already visited URLs to prevent duplicate fetching and infinite loops
class SeenURLStore {
private:
    // We use boolean as the value type just to mark presence
    HashMap<std::string, bool> map;
    mutable std::mutex mtx;

public:
    SeenURLStore();
    
    void markSeen(const std::string& url);
    bool isSeen(const std::string& url) const;
    int size() const;
};

#endif // SEEN_URL_STORE_H
