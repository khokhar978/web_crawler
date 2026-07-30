#include "HTTPClient.h"
#include <iostream>
#include <curl/curl.h>

// Callback function used by libcurl to save the downloaded bytes into our std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

HTTPClient::HTTPClient() {
    curlHandle = curl_easy_init();
    if (curlHandle) {
        CURL* curl = static_cast<CURL*>(curlHandle);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        // Wikipedia strongly prefers honest Bot User-Agents over fake browsers.
        // Fake browsers get hit by Cloudflare JS-Challenges.
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "SuperCodersBot/1.0 (contact@example.com)");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        // Set up callback globally for this handle
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    }
}

HTTPClient::~HTTPClient() {
    if (curlHandle) {
        curl_easy_cleanup(static_cast<CURL*>(curlHandle));
    }
}

std::string HTTPClient::fetchPage(const std::string& url) {
    if (!curlHandle) return "";
    
    CURL* curl = static_cast<CURL*>(curlHandle);
    std::string htmlBuffer = "";
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &htmlBuffer);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "Download failed for " << url << " - Error: " << curl_easy_strerror(res) << std::endl;
        return "";
    }
    
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCode != 200) {
        std::cerr << "HTTP Error " << httpCode << " for URL: " << url << std::endl;
        return "";
    }
    
    return htmlBuffer;
}
