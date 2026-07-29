#include <iostream>
#include <string>
#include <chrono>

#include "common/PageStorage.h"
#include "indexer/HTMLTextExtractor.h"
#include "indexer/Tokenizer.h"
#include "indexer/InvertedIndex.h"
#include "indexer/QueryEngine.h"

int main() {
    std::cout << "======================================\n";
    std::cout << "      SuperCoders Search Indexer      \n";
    std::cout << "======================================\n\n";

    // 1. Open Database
    std::cout << "Connecting to crawler storage (crawler.db & crawler_archive.dat)...\n";
    PageStorage storage("crawler_archive.dat", "crawler.db");
    
    int totalPages = storage.pageCount();
    if (totalPages == 0) {
        std::cout << "Database is empty! Run the crawler first.\n";
        return 0;
    }
    std::cout << "Found " << totalPages << " pages in database.\n\n";

    // 2. Initialize Index
    InvertedIndex index;
    std::cout << "Building Inverted Index...\n";
    
    auto startTime = std::chrono::high_resolution_clock::now();
    int processedCount = 0;
    
    // 3. Process every page
    // SQLite ROWIDs usually start at 1 and auto-increment
    int currentId = 1;
    
    while (processedCount < totalPages) {
        std::string url = storage.getURLByID(currentId);
        
        if (!url.empty()) {
            std::string html = storage.getPage(url);
            
            // Pipeline Step 1: Strip HTML tags, scripts, and styles
            std::string cleanText = HTMLTextExtractor::extractText(html);
            
            // Pipeline Step 2: Tokenize and normalize words, drop punctuation
            DynamicArray<std::string> tokens = Tokenizer::tokenize(cleanText);
            
            // Pipeline Step 3: Remove useless stop words to save massive memory
            Tokenizer::removeStopWords(tokens);
            
            // Pipeline Step 4: Feed clean tokens into the index (with Term Frequency)
            index.addDocument(currentId, tokens);
            
            processedCount++;
            
            // Print progress
            if (processedCount % 50 == 0 || processedCount == totalPages) {
                std::cout << "Processed " << processedCount << " / " << totalPages << " pages...\r";
                std::cout.flush();
            }
        }
        
        currentId++;
        
        // Safety break in case of massive database corruption or missing IDs
        if (currentId > totalPages * 5) {
            std::cout << "\nWarning: Stopped early due to missing IDs.\n";
            break;
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = endTime - startTime;
    
    std::cout << "\n\nIndexing Complete!\n";
    std::cout << "Time taken:      " << diff.count() << " seconds\n";
    std::cout << "Unique Keywords: " << index.size() << "\n";
    std::cout << "======================================\n";
    // 4. Interactive Search Engine Loop
    std::cout << "\nWelcome to SuperCoders Search!\n";
    std::cout << "Type a multi-word query to search (or 'exit' to quit).\n\n";
    
    std::string query;
    while (true) {
        std::cout << "Search> ";
        std::getline(std::cin, query);
        
        if (query == "exit" || query == "quit") {
            break;
        }
        if (query.empty()) {
            continue;
        }
        
        auto searchStart = std::chrono::high_resolution_clock::now();
        
        // Execute the Query
        DynamicArray<QueryResult> results = QueryEngine::search(query, index);
        
        auto searchEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> searchDiff = searchEnd - searchStart;
        
        // Display Results
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
    
    std::cout << "Shutting down Search Engine...\n";
    return 0;
}
