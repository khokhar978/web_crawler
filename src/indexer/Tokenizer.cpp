#include "indexer/Tokenizer.h"
#include "common/HashMap.h"
#include <cctype>

DynamicArray<std::string> Tokenizer::tokenize(const std::string& text) {
    DynamicArray<std::string> tokens;
    std::string currentToken;
    
    // We pre-allocate some capacity to avoid frequent reallocations. 
    // Average English word is ~5 chars, plus 1 space = ~6 chars per word.
    // However, DynamicArray handles its own capacity resizing gracefully.
    
    for (size_t i = 0; i < text.size(); ++i) {
        char c = text[i];
        
        // Alphanumeric characters form words.
        if (std::isalnum(static_cast<unsigned char>(c))) {
            currentToken += std::tolower(static_cast<unsigned char>(c));
        } else {
            // Any non-alphanumeric character (whitespace, punctuation) acts as a word boundary.
            if (!currentToken.empty()) {
                tokens.append(currentToken);
                currentToken.clear();
            }
        }
    }
    
    // Append the final token if the text doesn't end with punctuation/whitespace
    if (!currentToken.empty()) {
        tokens.append(currentToken);
    }
    
    return tokens;
}

void Tokenizer::removeStopWords(DynamicArray<std::string>& tokens) {
    // Static hash map so we only initialize it once across the entire crawler lifecycle
    static HashMap<std::string, bool> stopWords;
    static bool initialized = false;
    
    if (!initialized) {
        // Core English stop words list
        const char* words[] = {
            "a", "about", "above", "after", "again", "against", "all", "am", "an", "and", "any", "are", 
            "arent", "as", "at", "be", "because", "been", "before", "being", "below", "between", "both", 
            "but", "by", "cant", "cannot", "could", "couldnt", "did", "didnt", "do", "does", "doesnt", 
            "doing", "dont", "down", "during", "each", "few", "for", "from", "further", "had", "hadnt", 
            "has", "hasnt", "have", "havent", "having", "he", "hed", "hell", "hes", "her", "here", 
            "heres", "hers", "herself", "him", "himself", "his", "how", "hows", "i", "id", "ill", 
            "im", "ive", "if", "in", "into", "is", "isnt", "it", "its", "itself", "lets", 
            "me", "more", "most", "mustnt", "my", "myself", "no", "nor", "not", "of", "off", "on", 
            "once", "only", "or", "other", "ought", "our", "ours", "ourselves", "out", "over", "own", 
            "same", "shant", "she", "shed", "shell", "shes", "should", "shouldnt", "so", "some", 
            "such", "than", "that", "thats", "the", "their", "theirs", "them", "themselves", "then", 
            "there", "theres", "these", "they", "theyd", "theyll", "theyre", "theyve", "this", 
            "those", "through", "to", "too", "under", "until", "up", "very", "was", "wasnt", "we", 
            "wed", "well", "were", "weve", "werent", "what", "whats", "when", "whens", 
            "where", "wheres", "which", "while", "who", "whos", "whom", "why", "whys", "with", 
            "wont", "would", "wouldnt", "you", "youd", "youll", "youre", "youve", "your", "yours", 
            "yourself", "yourselves"
        };
        
        int numWords = sizeof(words) / sizeof(words[0]);
        for (int i = 0; i < numWords; ++i) {
            stopWords.put(words[i], true);
        }
        initialized = true;
    }
    
    DynamicArray<std::string> filteredTokens;
    for (int i = 0; i < tokens.size(); ++i) {
        // Use operator[] or get() - size() and get() are standard from DynamicArray interface
        std::string word = tokens[i];
        if (!stopWords.contains(word)) {
            filteredTokens.append(word);
        }
    }
    
    // Deep copy assignment operator replaces the old array
    tokens = filteredTokens;
}
