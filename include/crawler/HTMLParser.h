#pragma once
#include <string>
#include "DynamicArray.h"

class HTMLParser {
public:
    // Parses raw HTML and extracts all URLs found inside <a href="..."> tags.
    // We pass the baseUrl so we can convert relative links (like "/about.html") 
    // into absolute links (like "http://example.com/about.html").
    // If allowedDomain is provided, strictly filters out any links not matching the domain.
    static DynamicArray<std::string> extractLinks(const std::string& htmlContent, const std::string& baseUrl, const std::string& allowedDomain = "");
};
