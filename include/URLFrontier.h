#ifndef URL_FRONTIER_H
#define URL_FRONTIER_H

#include <string>
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

public:
    URLFrontier();
    
    void push(const std::string& url, int depth);
    FrontierEntry pop();
    bool isEmpty() const;
    int size() const;
};

#endif // URL_FRONTIER_H
