#include "indexer/HTMLTextExtractor.h"
#include <cctype>

// Helper to check case-insensitive match for tags like "<script" or "</style>"
static bool matchTag(const std::string& html, size_t pos, const std::string& tag) {
    if (pos + tag.size() > html.size()) {
        return false;
    }
    for (size_t i = 0; i < tag.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(html[pos + i])) != tag[i]) {
            return false;
        }
    }
    // Make sure it's bounded (e.g., "<script " or "<script>" but not "<scripts>")
    if (pos + tag.size() < html.size()) {
        char next = html[pos + tag.size()];
        if (std::isalnum(static_cast<unsigned char>(next))) {
            return false;
        }
    }
    return true;
}

std::string HTMLTextExtractor::extractText(const std::string& html) {
    std::string result;
    // Pre-allocate to prevent frequent reallocations, text is usually ~30% of HTML
    result.reserve(html.size() / 3);
    
    bool inTag = false;
    bool inScript = false;
    bool inStyle = false;
    bool inTitle = false;
    std::string titleText;
    
    for (size_t i = 0; i < html.size(); ++i) {
        char c = html[i];
        
        // Handle script block closing
        if (inScript) {
            if (c == '<' && matchTag(html, i, "</script>")) {
                inScript = false;
                inTag = false;
                i += 8; // skip "</script>"
            }
            continue;
        }
        
        // Handle style block closing
        if (inStyle) {
            if (c == '<' && matchTag(html, i, "</style>")) {
                inStyle = false;
                inTag = false;
                i += 7; // skip "</style>"
            }
            continue;
        }
        
        // Detect start of any tag
        if (c == '<') {
            if (matchTag(html, i, "<script")) {
                inScript = true;
            } else if (matchTag(html, i, "<style")) {
                inStyle = true;
            } else if (matchTag(html, i, "<title")) {
                inTitle = true;
            } else if (inTitle && matchTag(html, i, "</title>")) {
                inTitle = false;
            }
            inTag = true;
            continue;
        }
        
        // Skip characters inside regular tags
        if (inTag) {
            if (c == '>') {
                inTag = false;
                // Add a space to prevent joining words like <p>Hello</p><p>World</p> -> HelloWorld
                if (!result.empty() && result.back() != ' ') {
                    result += ' ';
                }
            }
            continue;
        }
        
        // We are outside of any tags, scripts, or styles - capture the visible text
        if (std::isspace(static_cast<unsigned char>(c))) {
            // Collapse multiple spaces/newlines into a single space
            if (!result.empty() && result.back() != ' ') {
                result += ' ';
                if (inTitle) titleText += ' ';
            }
        } else {
            result += c;
            if (inTitle) titleText += c;
        }
    }
    
    // Trim trailing space if one was added at the very end
    if (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    
    // Title Boosting Algorithm: Inject title words 10 extra times
    if (!titleText.empty()) {
        for (int j = 0; j < 10; ++j) {
            result += " " + titleText;
        }
    }
    
    return result;
}
