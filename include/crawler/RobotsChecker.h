#ifndef ROBOTS_CHECKER_H
#define ROBOTS_CHECKER_H

#include <string>
#include <vector>
#include "HashMap.h"
#include "DynamicArray.h"

// Stores the parsed rules from a single domain's robots.txt
struct RobotsRules {
    DynamicArray<std::string> disallowedPaths;
};

// Checks robots.txt compliance with a per-domain cache.
// Each domain's robots.txt is fetched exactly ONCE and cached in memory.
class RobotsChecker {
private:
    // Cache: domain -> list of disallowed paths
    // Using our custom HashMap from Project 1 for O(1) lookups
    HashMap<std::string, RobotsRules> cache;

    // Extracts the domain (scheme + host) from a full URL
    // e.g., "https://example.com/page?q=1" -> "https://example.com"
    static std::string extractDomain(const std::string& url);

    // Extracts just the path portion from a URL
    // e.g., "https://example.com/admin/panel" -> "/admin/panel"
    static std::string extractPath(const std::string& url);

    // Downloads and parses a robots.txt file, returning the rules
    static RobotsRules fetchAndParse(const std::string& domain);

public:
    RobotsChecker();

    // Returns true if the URL is allowed to be crawled.
    // Fetches and caches robots.txt for the domain on first encounter.
    bool isAllowed(const std::string& url);
};

#endif // ROBOTS_CHECKER_H
