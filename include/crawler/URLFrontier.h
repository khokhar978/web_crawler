#ifndef URL_FRONTIER_H
#define URL_FRONTIER_H

#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "DynamicArray.h"
#include "LinkedList.h"

// The struct that travels through the frontier queue
struct FrontierEntry {
    std::string url;
    int depth;
};

// Manages the list of URLs waiting to be crawled in a FIFO queue
class URLFrontier {
private:
    LinkedList<FrontierEntry> queue;
    mutable std::mutex mtx;
    std::condition_variable cv;

public:
    int totalWorkers{0};
    int waitingWorkers{0};
    bool isFinished{false};

    URLFrontier();
    
    void push(const std::string& url, int depth);
    FrontierEntry pop();
    bool isEmpty() const;
    int size() const;
    
    void incrementWorkers();
    void decrementWorkers();
    void resetFinished();
    
    // Persistence Methods
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    DynamicArray<std::string> getQueuedUrls() const;
};

#endif // URL_FRONTIER_H
