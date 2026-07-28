#include "HTTPClient.h"
#include <iostream>
#include <curl/curl.h>

// Callback function used by libcurl to save the downloaded bytes into our std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

std::string HTTPClient::fetchPage(const std::string& url) {
    std::string htmlBuffer = "";
    
    // Initialize a libcurl handle
    CURL* curl = curl_easy_init();
    if (curl) {
        // Set the URL we want to download
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        // Follow redirects (e.g., if http://google.com redirects to https://www.google.com)
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        // Disable SSL certificate verification (Windows libcurl often lacks the CA bundle)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        // Add a standard Browser User-Agent header to prevent aggressive anti-bot blocking
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/115.0.0.0 Safari/537.36");
        
        // Define our callback function so libcurl knows how to save the data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        
        // Pass our string buffer to the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlBuffer);
        
        // Set a reasonable timeout so a broken server doesn't freeze our crawler forever (increased to 30s)
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        
        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "Download failed for " << url << " - Error: " << curl_easy_strerror(res) << std::endl;
            htmlBuffer = ""; // Return empty string on failure as per design
        }
        
        // Clean up the handle
        curl_easy_cleanup(curl);
    }
    
    return htmlBuffer;
}
