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
    // 1. Parse Command Line Arguments (Optional)
    std::string seedUrl = "";
    int maxDepth = 2; // Default max depth

    if (argc >= 3) {
        seedUrl = argv[1];
        maxDepth = std::stoi(argv[2]);
        std::cout << "Seed URL from args: " << seedUrl << "\n";
        std::cout << "Max Depth from args: " << maxDepth << "\n\n";
    } else {
        std::cout << "No seed URL provided in arguments. Defaulting to interactive mode.\n\n";
    }

    std::cout << "Starting Web Crawler...\n";

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
    if (!seedUrl.empty()) {
        if (!seenStore.isSeen(seedUrl)) {
            frontier.push(seedUrl, 0);
        } else {
            std::cout << "Initial Seed URL has already been crawled in a previous session!\n";
        }
    }

    // --- Load frontier backup ---
    // (We do NOT mark these as seen in SeenStore, because our pop() logic relies on isSeen() to skip duplicates!)
    frontier.loadFromFile("frontier_backup.txt");
    // ----------------------------

    int pagesCrawled = 0;

    // 4. Interactive Crawler Outer Loop
    while (true) {
        // If frontier runs dry, prompt the user for more work
        if (frontier.isEmpty()) {
            std::cout << "\nFrontier is empty. Enter a new seed URL (or type 'exit' to quit): ";
            std::string inputUrl;
            std::cin >> inputUrl;

            if (inputUrl == "exit") {
                break;
            }

            if (!seenStore.isSeen(inputUrl)) {
                frontier.push(inputUrl, 0);
            } else {
                std::cout << "That URL has already been crawled! Please try another.\n";
                continue;
            }
        }

        // 5. Main Crawling Loop (Breadth-First Search)
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
            
            pagesCrawled++;
            
            // Save frontier state to disk periodically to guarantee zero data loss with high performance
            if (pagesCrawled % 50 == 0) {
                frontier.saveToFile("frontier_backup.txt");
            }
        }
    }

    std::cout << "\nCrawling Complete. Exiting crawler loop.\n";

    // 5. Run the Indexer Stub for Project 3 preparation
    runIndexerStub(storage);

    return 0;
}
