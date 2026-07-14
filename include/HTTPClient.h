#pragma once
#include <string>

// Abstract Downloader Interface (per Phase 0 Proposal)
class Downloader {
public:
    virtual ~Downloader() = default;
    virtual std::string fetchPage(const std::string& url) = 0;
};

// libcurl implementation
class HTTPClient : public Downloader {
public:
    // Fetches the raw HTML content from the given URL.
    // Returns an empty string if the download fails (404/500 errors).
    std::string fetchPage(const std::string& url) override;
};
