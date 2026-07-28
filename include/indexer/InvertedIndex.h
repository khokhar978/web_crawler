#pragma once

#include <string>
#include "common/DynamicArray.h"
#include "common/HashMap.h"

// Represents a document and how many times a specific word appeared in it
struct IndexPosting {
    int docID;
    int frequency;
    
    // Default constructor needed for DynamicArray initialization
    IndexPosting() : docID(-1), frequency(0) {}
    IndexPosting(int id, int freq) : docID(id), frequency(freq) {}
};

class InvertedIndex {
private:
    // Core data structure: Maps a keyword to a list of Postings (DocID + Frequency)
    HashMap<std::string, DynamicArray<IndexPosting>> indexMap;
    
public:
    InvertedIndex();
    
    /**
     * Integrates all tokens from a specific document into the inverted index.
     * Records the document ID and calculates the term frequency within that document.
     */
    void addDocument(int docID, const DynamicArray<std::string>& tokens);
    
    /**
     * Look up a keyword in O(1) average time.
     * Returns an array of IndexPostings (containing Document IDs and Frequencies).
     */
    DynamicArray<IndexPosting> search(const std::string& query) const;
    
    /**
     * Returns the total number of unique words currently in the index.
     */
    int size() const;
};
