#include "URLFrontier.h"

URLFrontier::URLFrontier() {
    // Constructor
}

void URLFrontier::push(const std::string& url, int depth) {
    // Create a new FrontierEntry struct
    FrontierEntry entry = {url, depth};
    // Append it to the back of our LinkedList queue
    queue.append(entry);
}

FrontierEntry URLFrontier::pop() {
    // 1. Get the entry at the very front of the queue (index 0)
    FrontierEntry frontEntry = queue.get(0);
    
    // 2. Remove that entry from the LinkedList to advance the queue
    queue.removeFirst();
    
    // 3. Return the copied entry to the caller
    return frontEntry;
}

bool URLFrontier::isEmpty() const {
    return queue.isEmpty();
}

int URLFrontier::size() const {
    return queue.getSize();
}
