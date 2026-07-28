#pragma once

#include <string>

class HTMLTextExtractor {
public:
    /**
     * Parses raw HTML and removes all tags (e.g., <script>, <div>),
     * returning only the clean, searchable visible text.
     */
    static std::string extractText(const std::string& html);
};
