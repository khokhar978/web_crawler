#include <iostream>
#include <string>
#include <chrono>

#include "common/PageStorage.h"
#include "indexer/HTMLTextExtractor.h"
#include "indexer/Tokenizer.h"
#include "indexer/InvertedIndex.h"
#include "indexer/QueryEngine.h"
#include <thread>
#include <atomic>

int main(int argc, char* argv[]) {
    bool isDaemon = (argc > 1 && std::string(argv[1]) == "--daemon");
    
    if (!isDaemon) {
        std::cout << "======================================\n";
        std::cout << "      SuperCoders Search Indexer      \n";
        std::cout << "======================================\n\n";
    }

    // 1. Open Database
    std::cout << "Connecting to crawler storage (crawler.db & crawler_archive.dat)...\n";
    PageStorage storage("crawler_archive.dat", "crawler.db");
    
    int totalPages = storage.pageCount();
    if (totalPages == 0) {
        std::cout << "Database is empty! Run the crawler first.\n";
        return 0;
    }
    std::cout << "Found " << totalPages << " pages in database.\n\n";

    InvertedIndex index;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "Attempting to load Inverted Index from disk...\n";
    if (index.loadFromDisk("inverted_index.dat")) {
        std::cout << "Successfully loaded Inverted Index from disk!\n";
    } else {
        std::cout << "No saved index found. Building Inverted Index from database...\n";
        
        int processedCount = 0;
        // SQLite ROWIDs usually start at 1 and auto-increment
        int currentId = 1;
        
        // Determine optimal thread count based on hardware
        int numThreads = std::thread::hardware_concurrency();
        if (numThreads == 0) numThreads = 8;
        std::cout << "Using " << numThreads << " parallel threads for indexing...\n";
        
        int chunkSize = 800; // Large chunk to minimize thread lifecycle overhead
        
        // --- DOUBLE BUFFERING PRODUCER-CONSUMER PIPELINE ---
        
        // Pre-fetch the very first batch (Buffer A)
        DynamicArray<PageData> currentBatch = storage.getPageBatch(currentId, chunkSize);
        if (!currentBatch.isEmpty()) {
            currentId = currentBatch.get(currentBatch.size() - 1).id + 1;
        } else {
            currentId++;
        }
        
        while (!currentBatch.isEmpty()) {
            // 1. Start the 9th Producer Thread to concurrently fetch the NEXT batch (Buffer B) from disk
            DynamicArray<PageData> nextBatch;
            std::thread diskThread([&storage, &nextBatch, currentId, chunkSize]() {
                nextBatch = storage.getPageBatch(currentId, chunkSize);
            });
            
            // 2. Dynamic Load Balancing: Shared atomic index for the 8 CPU workers
            std::atomic<int> next_page_idx(0);
            
            DynamicArray<std::thread> workers(numThreads);
            
            for (int t = 0; t < numThreads; ++t) {
                // 3. Spawn CPU worker thread
                workers.append(std::thread([&index, &currentBatch, &next_page_idx]() {
                    while (true) {
                        // Instantly grab the next available page index
                        int p = next_page_idx.fetch_add(1, std::memory_order_relaxed);
                        
                        // If there are no pages left in the batch, this thread is done
                        if (p >= currentBatch.size()) break;
                        
                        const std::string& html = currentBatch.get(p).html;
                        int docId = currentBatch.get(p).id;
                        
                        std::string cleanText = HTMLTextExtractor::extractText(html);
                        DynamicArray<std::string> tokens = Tokenizer::tokenize(cleanText);
                        Tokenizer::removeStopWords(tokens);
                        
                        index.addDocument(docId, tokens);
                    }
                }));
            }
            
            // 4. Wait for all 8 CPU workers to finish processing currentBatch
            for (auto& t : workers) {
                if (t.joinable()) {
                    t.join();
                }
            }
            
            processedCount += currentBatch.size();
            
            // Print progress
            if (processedCount % 100 == 0 || processedCount >= totalPages) {
                std::cout << "Processed " << processedCount << " / " << totalPages << " pages...\r";
                std::cout.flush();
            }
            
            // 5. Ensure the 9th Producer Thread has finished loading nextBatch
            if (diskThread.joinable()) {
                diskThread.join();
            }
            
            // 6. Swap Buffer B into Buffer A (instant move operation)
            currentBatch = std::move(nextBatch);
            
            // Update currentId for the next loop
            if (!currentBatch.isEmpty()) {
                currentId = currentBatch.get(currentBatch.size() - 1).id + 1;
            } else if (currentId <= totalPages * 5) {
                // If it was empty but we haven't hit the safety limit, skip missing ID
                currentId++;
                // Manually fetch next one to continue loop
                currentBatch = storage.getPageBatch(currentId, chunkSize);
                if (!currentBatch.isEmpty()) {
                    currentId = currentBatch.get(currentBatch.size() - 1).id + 1;
                }
            }
            
            if (currentId > totalPages * 5) {
                if (processedCount < totalPages) {
                    std::cout << "\nWarning: Stopped early due to missing IDs.\n";
                }
                break;
            }
        }
        
        std::cout << "\nSaving Inverted Index to disk...\n";
        index.saveToDisk("inverted_index.dat");
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = endTime - startTime;
    
    std::cout << "\n\nIndexing Complete!\n";
    std::cout << "Time taken:      " << diff.count() << " seconds\n";
    std::cout << "Unique Keywords: " << index.size() << "\n";
    std::cout << "======================================\n";
    // 4. Interactive Search Engine Loop
    if (!isDaemon) {
        std::cout << "\nWelcome to SuperCoders Search!\n";
        std::cout << "Type a multi-word query to search (or 'exit' to quit).\n\n";
    }
    
    std::string query;
    while (true) {
        if (!isDaemon) std::cout << "Search> ";
        if (!std::getline(std::cin, query)) break; // Exit if stdin closes (e.g. Node process dies)
        
        if (!isDaemon && (query == "exit" || query == "quit")) {
            break;
        }
        if (query.empty()) {
            if (isDaemon) {
                std::cout << "[]\n";
                std::cout.flush();
            }
            continue;
        }
        
        auto searchStart = std::chrono::high_resolution_clock::now();
        
        // Execute the Query
        DynamicArray<QueryResult> results = QueryEngine::search(query, index);
        
        auto searchEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> searchDiff = searchEnd - searchStart;
        
        if (isDaemon) {
            // Output JSON for the Node.js API
            std::cout << "[";
            int displayCount = (results.size() < 10) ? results.size() : 10;
            for (int i = 0; i < displayCount; ++i) {
                std::string resultUrl = storage.getURLByID(results[i].docID);
                std::cout << "{\"url\":\"" << resultUrl << "\",\"score\":" << results[i].score << "}";
                if (i < displayCount - 1) std::cout << ",";
            }
            std::cout << "]\n";
            std::cout.flush();
        } else {
            // Display Results for Human CLI
            if (results.isEmpty()) {
                std::cout << "No matching documents found.\n\n";
            } else {
                std::cout << "Found " << results.size() << " results in " << searchDiff.count() << " ms:\n";
                
                // Print top 5 results
                int displayCount = (results.size() < 5) ? results.size() : 5;
                for (int i = 0; i < displayCount; ++i) {
                    std::string resultUrl = storage.getURLByID(results[i].docID);
                    std::cout << "  " << (i + 1) << ". [Score: " << results[i].score << "] " << resultUrl << "\n";
                }
                std::cout << "\n";
            }
        }
    }
    
    std::cout << "Shutting down Search Engine...\n";
    return 0;
}
