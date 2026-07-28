#include "SeenURLStore.h"

SeenURLStore::SeenURLStore() {
    // Constructor initializes the internal HashMap automatically
}

void SeenURLStore::markSeen(const std::string& url) {
    // Insert the URL into the HashMap with a boolean 'true' to mark it as visited
    map.put(url, true);
}

bool SeenURLStore::isSeen(const std::string& url) const {
    // Return true if the HashMap contains this URL
    return map.contains(url);
}

int SeenURLStore::size() const {
    // Return the total number of unique URLs we have tracked
    return map.size();
}
