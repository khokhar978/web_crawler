#pragma once

#include <string>
#include "common/DynamicArray.h"

class Tokenizer {
public:
    /**
     * Splits plain text into individual tokens.
     * Normalizes by converting to lowercase and stripping punctuation.
     */
    static DynamicArray<std::string> tokenize(const std::string& text);
    
    /**
     * Removes common English stop words (e.g., "the", "and") from the token array.
     */
    static void removeStopWords(DynamicArray<std::string>& tokens);
};
