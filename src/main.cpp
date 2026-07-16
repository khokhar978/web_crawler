#include <iostream>
#include <string>
#include "URLFrontier.h"
#include "SeenURLStore.h"
#include "PageStorage.h"
#include "HTMLParser.h"
#include "HTTPClient.h"

// A simple stub function to demonstrate iteration over the PageStorage database for Project 3
void runIndexerStub(PageStorage& storage) {
    std::cout << "\n============================================\n";
    std::cout << "Running Project 3 Indexer Preparation Stub...\n";
    std::cout << "============================================\n";
    int count = storage.pageCount();
    std::cout << "Total pages archived: " << count << "\n";
    
    for (int i = 1; i <= count; ++i) { // Assuming SQLite IDs start at 1
        std::string url = storage.getURLByID(i);
        std::cout << "[" << i << "] -> " << url << "\n";
    }
    std::cout << "============================================\n\n";
}

int main(int argc, char* argv[]) {
    // 1. Parse Command Line Arguments
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <seed_url> <max_depth>\n";
        std::cerr << "Example: " << argv[0] << " http://example.com 2\n";
        return 1;
    }

    std::string seedUrl = argv[1];
    int maxDepth = std::stoi(argv[2]);

    std::cout << "Starting Web Crawler...\n";
    std::cout << "Seed URL: " << seedUrl << "\n";
    std::cout << "Max Depth: " << maxDepth << "\n\n";

    // 2. Initialize Core Subsystems
    URLFrontier frontier;
    SeenURLStore seenStore;
    PageStorage storage("crawler_archive.dat", "crawler.db");
    HTTPClient downloader;

    // --- Hydrate SeenStore from Database to skip duplicates on restart ---
    int existingCount = storage.pageCount();
    if (existingCount > 0) {
        std::cout << "Resuming session. Loading " << existingCount << " previously crawled URLs from database...\n";
        for (int i = 1; i <= existingCount; ++i) {
            std::string url = storage.getURLByID(i);
            if (!url.empty()) {
                seenStore.markSeen(url);
            }
        }
    }
    // -------------------------------------------------------------------

    // 3. Push initial seed
    if (!seenStore.isSeen(seedUrl)) {
        frontier.push(seedUrl, 0);
    } else {
        std::cout << "Seed URL has already been crawled in a previous session!\n";
    }

    // 4. Main Crawling Loop (Breadth-First Search)
    while (!frontier.isEmpty()) {
        FrontierEntry current = frontier.pop();
        std::string currentUrl = current.url;
        int currentDepth = current.depth;

        // Skip if we have already crawled this URL
        if (seenStore.isSeen(currentUrl)) {
            continue;
        }

        // Mark it as seen
        seenStore.markSeen(currentUrl);
        std::cout << "Crawling [Depth " << currentDepth << "]: " << currentUrl << "...\n";

        // Download the page
        std::string htmlContent = downloader.fetchPage(currentUrl);
        if (htmlContent.empty()) {
            std::cout << "  -> Failed to download or empty page. Skipping.\n";
            continue;
        }

        // Store the page in SQLite and append to archive
        storage.storePage(currentUrl, htmlContent, currentDepth);
        std::cout << "  -> Downloaded and stored (" << htmlContent.size() << " bytes).\n";

        // If we haven't reached the max depth, extract links and add them to the frontier
        if (currentDepth < maxDepth) {
            DynamicArray<std::string> newLinks = HTMLParser::extractLinks(htmlContent, currentUrl);
            int numLinks = newLinks.size();
            std::cout << "  -> Extracted " << numLinks << " links.\n";

            for (int i = 0; i < numLinks; ++i) {
                // Avoid flooding the frontier with already-seen URLs
                if (!seenStore.isSeen(newLinks[i])) {
                    frontier.push(newLinks[i], currentDepth + 1);
                }
            }
        }
    }

    std::cout << "\nCrawling Complete. Frontier is empty.\n";

    // 5. Run the Indexer Stub for Project 3 preparation
    runIndexerStub(storage);

    return 0;
}
