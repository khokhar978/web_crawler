#include "URLFrontier.h"
#include <fstream>
#include "Logger.h"

URLFrontier::URLFrontier() {
    // Constructor
}

void URLFrontier::push(const std::string& url, int depth) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.append({url, depth});
    cv.notify_one();
}

FrontierEntry URLFrontier::pop() {
    std::unique_lock<std::mutex> lock(mtx);
    
    waitingWorkers++;
    
    // If all workers are now waiting and the queue is empty, the crawler is completely idle!
    if (waitingWorkers == totalWorkers && queue.isEmpty()) {
        isFinished = true;
        cv.notify_all();
    }
    
    cv.wait(lock, [this]() { return !queue.isEmpty() || isFinished; });
    
    waitingWorkers--;
    
    if (isFinished && queue.isEmpty()) {
        return FrontierEntry{"", -1}; // Terminal signal
    }
    
    FrontierEntry frontEntry = queue.get(0);
    queue.removeFirst();
    return frontEntry;
}

bool URLFrontier::isEmpty() const {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.isEmpty();
}

int URLFrontier::size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.getSize();
}

void URLFrontier::incrementWorkers() {
    std::lock_guard<std::mutex> lock(mtx);
    totalWorkers++;
}

void URLFrontier::decrementWorkers() {
    std::lock_guard<std::mutex> lock(mtx);
    totalWorkers--;
}

void URLFrontier::resetFinished() {
    std::lock_guard<std::mutex> lock(mtx);
    isFinished = false;
}

void URLFrontier::saveToFile(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(mtx);
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
        return;
    }
    
    int depth;
    std::string url;
    int count = 0;
    while (inFile >> depth >> url) {
        push(url, depth); // push already locks internally
        count++;
    }
    inFile.close();
    
    if (count > 0) {
        Logger::info("Loaded " + std::to_string(count) + " URLs into the Frontier from backup.");
    }
}

DynamicArray<std::string> URLFrontier::getQueuedUrls() const {
    std::lock_guard<std::mutex> lock(mtx);
    int sz = queue.getSize();
    DynamicArray<std::string> urls(sz > 0 ? sz : 4);
    for (const auto& entry : queue) {
        urls.append(entry.url);
    }
    return urls;
}
