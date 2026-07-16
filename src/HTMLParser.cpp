#include "HTMLParser.h"
#include <iostream>

DynamicArray<std::string> HTMLParser::extractLinks(const std::string& htmlContent, const std::string& baseUrl) {
    DynamicArray<std::string> extractedUrls;
    
    size_t pos = 0;

    // Fast manual string search loop
    while ((pos = htmlContent.find("<a ", pos)) != std::string::npos) {
        // Find the href attribute within this anchor tag
        size_t hrefPos = htmlContent.find("href=\"", pos);
        
        // Find the end of this anchor tag so we don't accidentally grab a different tag's href
        size_t endTagPos = htmlContent.find(">", pos);

        // If href exists AND it is inside the current <a> tag
        if (hrefPos != std::string::npos && hrefPos < endTagPos) {
            // Move past "href=\"" to the start of the actual URL
            size_t startQuote = hrefPos + 6; 
            
            // Find the closing quote
            size_t endQuote = htmlContent.find("\"", startQuote);
            
            if (endQuote != std::string::npos) {
                std::string url = htmlContent.substr(startQuote, endQuote - startQuote);
                
                // Ignore empty URLs, javascript links, or internal page jumps (#)
                if (!url.empty() && url.find("javascript:") != 0 && url.find("mailto:") != 0 && url.find("#") != 0) {
                    
                    // Improved URL Normalization Resolver
                    if (url.find("http") != 0) {
                        if (url.find("//") == 0) {
                            // Protocol relative
                            size_t colonPos = baseUrl.find(":");
                            std::string scheme = (colonPos != std::string::npos) ? baseUrl.substr(0, colonPos + 1) : "http:";
                            url = scheme + url;
                        } else if (url.front() == '/') {
                            // Root relative
                            size_t schemeEnd = baseUrl.find("://");
                            size_t startPos = (schemeEnd != std::string::npos) ? schemeEnd + 3 : 0;
                            size_t domainEnd = baseUrl.find("/", startPos);
                            if (domainEnd != std::string::npos) {
                                url = baseUrl.substr(0, domainEnd) + url;
                            } else {
                                if (!baseUrl.empty() && baseUrl.back() == '/') {
                                    url = baseUrl.substr(0, baseUrl.length() - 1) + url;
                                } else {
                                    url = baseUrl + url;
                                }
                            }
                        } else if (url.front() == '?') {
                            // Query relative
                            size_t queryPos = baseUrl.find('?');
                            std::string baseWithoutQuery = (queryPos != std::string::npos) ? baseUrl.substr(0, queryPos) : baseUrl;
                            url = baseWithoutQuery + url;
                        } else {
                            // Path relative
                            size_t lastSlash = baseUrl.find_last_of('/');
                            size_t schemeEnd = baseUrl.find("://");
                            if (lastSlash != std::string::npos && (schemeEnd == std::string::npos || lastSlash > schemeEnd + 2)) {
                                url = baseUrl.substr(0, lastSlash + 1) + url;
                            } else if (!baseUrl.empty() && baseUrl.back() != '/') {
                                url = baseUrl + "/" + url;
                            } else {
                                url = baseUrl + url;
                            }
                        }
                    }
                    
                    // Add to our DynamicArray!
                    extractedUrls.append(url);
                }
            }
        }
        
        // Advance past this tag to keep searching
        pos = endTagPos;
        if (pos == std::string::npos) break; // Safety check
    }
    
    return extractedUrls;
}
