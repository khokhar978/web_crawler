#include "URLFrontier.h"
#include <fstream>
#include "Logger.h"

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

void URLFrontier::saveToFile(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        Logger::error("Failed to open " + filename + " for saving frontier.");
        return;
    }
    for (const auto& entry : queue) {
        outFile << entry.depth << " " << entry.url << "\n";
    }
    outFile.close();
}

void URLFrontier::loadFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        // It's normal if the file doesn't exist on the first run
        return;
    }
    
    int depth;
    std::string url;
    int count = 0;
    while (inFile >> depth >> url) {
        push(url, depth);
        count++;
    }
    inFile.close();
    
    if (count > 0) {
        Logger::info("Loaded " + std::to_string(count) + " URLs into the Frontier from backup.");
    }
}

std::vector<std::string> URLFrontier::getQueuedUrls() const {
    std::vector<std::string> urls;
    urls.reserve(queue.getSize());
    for (const auto& entry : queue) {
        urls.push_back(entry.url);
    }
    return urls;
}
