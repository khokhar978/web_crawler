#include "RobotsChecker.h"
#include "HTTPClient.h"
#include "Logger.h"
#include <sstream>

RobotsChecker::RobotsChecker() : cache(64) {
    // Initialize with a reasonably sized HashMap (64 buckets for domains)
}

std::string RobotsChecker::extractDomain(const std::string& url) {
    // Find the protocol separator "://"
    size_t protocolEnd = url.find("://");
    if (protocolEnd == std::string::npos) {
        return "";
    }
    
    // Find the first '/' after the protocol (start of the path)
    size_t pathStart = url.find('/', protocolEnd + 3);
    if (pathStart == std::string::npos) {
        return url; // No path, the whole URL is the domain
    }
    
    return url.substr(0, pathStart);
}

std::string RobotsChecker::extractPath(const std::string& url) {
    size_t protocolEnd = url.find("://");
    if (protocolEnd == std::string::npos) {
        return "/";
    }
    
    size_t pathStart = url.find('/', protocolEnd + 3);
    if (pathStart == std::string::npos) {
        return "/";
    }
    
    // Strip query string for path matching
    size_t queryStart = url.find('?', pathStart);
    if (queryStart != std::string::npos) {
        return url.substr(pathStart, queryStart - pathStart);
    }
    
    return url.substr(pathStart);
}

RobotsRules RobotsChecker::fetchAndParse(const std::string& domain) {
    RobotsRules rules;
    
    // Download robots.txt from the domain
    HTTPClient client;
    std::string robotsUrl = domain + "/robots.txt";
    std::string content = client.fetchPage(robotsUrl);
    
    if (content.empty()) {
        // If robots.txt doesn't exist or fails to download, allow everything
        Logger::info("No robots.txt found for " + domain + ". All paths allowed.");
        return rules;
    }
    
    Logger::info("Parsing robots.txt for " + domain);
    
    // Parse the robots.txt content line by line
    // We only care about rules that apply to User-agent: * (all bots)
    std::istringstream stream(content);
    std::string line;
    bool appliesToUs = false;
    
    while (std::getline(stream, line)) {
        // Strip carriage return if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Check for User-agent directive
        if (line.substr(0, 11) == "User-agent:" || line.substr(0, 11) == "user-agent:") {
            std::string agent = line.substr(11);
            // Trim leading whitespace
            size_t start = agent.find_first_not_of(" \t");
            if (start != std::string::npos) {
                agent = agent.substr(start);
            }
            
            // We respond to the wildcard agent "*"
            appliesToUs = (agent == "*");
            continue;
        }
        
        // Only process Disallow rules that apply to us
        if (appliesToUs) {
            if (line.substr(0, 9) == "Disallow:" || line.substr(0, 9) == "disallow:") {
                std::string path = line.substr(9);
                // Trim leading whitespace
                size_t start = path.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    path = path.substr(start);
                }
                
                // An empty Disallow means allow everything
                if (!path.empty()) {
                    rules.disallowedPaths.append(path);
                }
            }
        }
    }
    
    Logger::info("Found " + std::to_string(rules.disallowedPaths.size()) + " disallowed paths for " + domain);
    return rules;
}

bool RobotsChecker::isAllowed(const std::string& url) {
    std::string domain = extractDomain(url);
    if (domain.empty()) {
        return true; // Can't determine domain, allow by default
    }
    
    // Check if we've already cached this domain's rules
    if (!cache.contains(domain)) {
        // First time seeing this domain — fetch and cache its robots.txt
        RobotsRules rules = fetchAndParse(domain);
        cache.put(domain, rules);
    }
    
    // Get the cached rules
    RobotsRules& rules = cache.get(domain);
    std::string path = extractPath(url);
    
    // Check if the path starts with any disallowed prefix
    for (int i = 0; i < rules.disallowedPaths.size(); ++i) {
        const std::string& disallowed = rules.disallowedPaths[i];
        // robots.txt uses prefix matching
        if (path.substr(0, disallowed.size()) == disallowed) {
            Logger::debug("BLOCKED by robots.txt: " + url + " (matched rule: " + disallowed + ")");
            return false;
        }
    }
    
    return true;
}
