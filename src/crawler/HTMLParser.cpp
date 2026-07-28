#include "HTMLParser.h"
#include <iostream>
#include <cctype>

DynamicArray<std::string> HTMLParser::extractLinks(const std::string& htmlContent, const std::string& baseUrl) {
    DynamicArray<std::string> extractedUrls;
    
    size_t pos = 0;

    // Fast manual string search loop
    while ((pos = htmlContent.find('<', pos)) != std::string::npos) {
        // Check if this is an 'a' or 'A' tag
        if (pos + 1 >= htmlContent.length() || std::tolower(static_cast<unsigned char>(htmlContent[pos + 1])) != 'a') {
            pos++;
            continue;
        }
        
        // Ensure it's isolated (space, tab, newline, or immediately closing)
        char nextC = pos + 2 < htmlContent.length() ? htmlContent[pos + 2] : ' ';
        if (!std::isspace(static_cast<unsigned char>(nextC)) && nextC != '>') {
            pos++;
            continue;
        }

        // Find the end of this anchor tag so we don't accidentally grab a different tag's href
        size_t endTagPos = htmlContent.find('>', pos);
        if (endTagPos == std::string::npos) break;

        // Find href case-insensitively within this tag
        size_t hrefPos = std::string::npos;
        for (size_t i = pos; i + 4 < endTagPos; ++i) {
            if (std::tolower(static_cast<unsigned char>(htmlContent[i])) == 'h' &&
                std::tolower(static_cast<unsigned char>(htmlContent[i+1])) == 'r' &&
                std::tolower(static_cast<unsigned char>(htmlContent[i+2])) == 'e' &&
                std::tolower(static_cast<unsigned char>(htmlContent[i+3])) == 'f' &&
                htmlContent[i+4] == '=') {
                hrefPos = i;
                break;
            }
        }

        // If href exists AND it is inside the current <a> tag
        if (hrefPos != std::string::npos) {
            // Move past "href=" to the value
            size_t valStart = hrefPos + 5;
            
            // Skip any spaces between '=' and the quote
            while (valStart < endTagPos && std::isspace(static_cast<unsigned char>(htmlContent[valStart]))) {
                valStart++;
            }

            char quoteChar = htmlContent[valStart];
            // Check for both single and double quotes
            if (quoteChar == '"' || quoteChar == '\'') {
                size_t startQuote = valStart + 1; 
                
                // Find the matching closing quote
                size_t endQuote = htmlContent.find(quoteChar, startQuote);
                
                if (endQuote != std::string::npos && endQuote < endTagPos) {
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
        }
        
        // Advance past this tag to keep searching
        pos = endTagPos;
        if (pos == std::string::npos) break; // Safety check
    }
    
    return extractedUrls;
}
