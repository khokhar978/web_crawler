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
                    
                    // Rudimentary Relative URL Resolver
                    // If the URL doesn't start with http, it is likely relative
                    if (url.find("http") != 0) {
                        // Ensure clean connection between base and relative path
                        if (!baseUrl.empty() && baseUrl.back() == '/' && url.front() == '/') {
                            url = baseUrl + url.substr(1);
                        } else if (!baseUrl.empty() && baseUrl.back() != '/' && url.front() != '/') {
                            url = baseUrl + "/" + url;
                        } else {
                            url = baseUrl + url;
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
