#include <iostream>
#include <string>
#include "URLFrontier.h"
#include "SeenURLStore.h"
#include "PageStorage.h"
#include "HTMLParser.h"
#include "HTTPClient.h"
#include "Logger.h"
#include "RobotsChecker.h"

// A simple stub function to demonstrate iteration over the PageStorage database for Project 3
void runIndexerStub(PageStorage& storage) {
    Logger::info("============================================");
    Logger::info("Running Project 3 Indexer Preparation Stub...");
    Logger::info("============================================");
    int count = storage.pageCount();
    Logger::info("Total pages archived: " + std::to_string(count));
    
    for (int i = 1; i <= count; ++i) { // Assuming SQLite IDs start at 1
        std::string url = storage.getURLByID(i);
        Logger::info("[" + std::to_string(i) + "] -> " + url);
    }
    Logger::info("============================================");
}

int main(int argc, char* argv[]) {
    // Initialize Logger (appends to crawler.log)
    Logger::init("crawler.log");

    // 1. Parse Command Line Arguments (Optional)
    std::string seedUrl = "";
    int maxDepth = 2; // Default max depth

    if (argc >= 3) {
        seedUrl = argv[1];
        maxDepth = std::stoi(argv[2]);
        Logger::info("Seed URL from args: " + seedUrl);
        Logger::info("Max Depth from args: " + std::to_string(maxDepth));
    } else {
        Logger::info("No seed URL provided in arguments. Defaulting to interactive mode.");
    }

    Logger::info("Starting Web Crawler...");

    // 2. Initialize Core Subsystems
    URLFrontier frontier;
    SeenURLStore seenStore;
    PageStorage storage("crawler_archive.dat", "crawler.db");
    HTTPClient downloader;
    RobotsChecker robotsChecker;

    // --- Hydrate SeenStore from Database to skip duplicates on restart ---
    int existingCount = storage.pageCount();
    if (existingCount > 0) {
        Logger::info("Resuming session. Loading " + std::to_string(existingCount) + " previously crawled URLs from database...");
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
            Logger::warn("Initial Seed URL has already been crawled in a previous session!");
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
            Logger::info("Frontier is empty. Waiting for user input...");
            std::cout << "\nFrontier is empty. Enter a new seed URL (or type 'exit' to quit): ";
            std::string inputUrl;
            std::cin >> inputUrl;

            if (inputUrl == "exit") {
                Logger::info("User requested exit.");
                break;
            }

            if (!seenStore.isSeen(inputUrl)) {
                frontier.push(inputUrl, 0);
                Logger::info("New seed URL added: " + inputUrl);
            } else {
                Logger::warn("User entered a URL that has already been crawled: " + inputUrl);
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

            // Check robots.txt compliance (cached per domain)
            if (!robotsChecker.isAllowed(currentUrl)) {
                Logger::warn("Skipping (blocked by robots.txt): " + currentUrl);
                continue;
            }

            Logger::info("Crawling [Depth " + std::to_string(currentDepth) + "]: " + currentUrl);

            // Download the page
            std::string htmlContent = downloader.fetchPage(currentUrl);
            if (htmlContent.empty()) {
                Logger::error("Failed to download or empty page: " + currentUrl);
                continue;
            }

            // Store the page in SQLite and append to archive
            storage.storePage(currentUrl, htmlContent, currentDepth);
            Logger::info("Downloaded and stored (" + std::to_string(htmlContent.size()) + " bytes): " + currentUrl);

            // If we haven't reached the max depth, extract links and add them to the frontier
            if (currentDepth < maxDepth) {
                DynamicArray<std::string> newLinks = HTMLParser::extractLinks(htmlContent, currentUrl);
                int numLinks = newLinks.size();
                Logger::info("Extracted " + std::to_string(numLinks) + " links from: " + currentUrl);

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
                Logger::debug("Frontier backup saved. Pages crawled so far: " + std::to_string(pagesCrawled));
            }
        }
    }

    Logger::info("Crawling Complete. Total pages crawled this session: " + std::to_string(pagesCrawled));

    // 6. Run the Indexer Stub for Project 3 preparation
    runIndexerStub(storage);

    Logger::shutdown();
    return 0;
}
