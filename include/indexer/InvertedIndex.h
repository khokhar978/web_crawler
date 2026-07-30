#pragma once

#include <string>
#include <mutex>
#include "common/DynamicArray.h"
#include "common/HashMap.h"

// Represents a document and how many times a specific word appeared in it
struct IndexPosting {
    int docID;
    int frequency;
    
    // Default constructor needed for DynamicArray initialization
    IndexPosting() : docID(-1), frequency(0) {}
    IndexPosting(int id, int freq) : docID(id), frequency(freq) {}
    
    // Sort by docID ascending
    bool operator<(const IndexPosting& other) const {
        return docID < other.docID;
    }
};

class InvertedIndex {
private:
    // Core data structure: Maps a keyword to a list of Postings (DocID + Frequency)
    HashMap<std::string, DynamicArray<IndexPosting>> indexMap;
    mutable std::mutex indexMutex;
    int maxDocID;
    
    // BM25 Variables
    DynamicArray<int> docLengths;
    long long totalTokens;
    
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
     * Sorts all postings by docID. Essential for multi-threaded indexing
     * where documents may be appended out of order.
     */
    void sortPostings();
    
    /**
     * Returns the total number of unique words currently in the index.
     */
    int size() const;
    
    /**
     * Returns the highest docID seen, representing the total number of documents.
     * Needed for BM25 calculations.
     */
    int getTotalDocs() const;
    
    /**
     * Returns the length (total tokens) of a specific document.
     */
    int getDocLength(int docID) const;
    
    /**
     * Returns the total number of tokens across all documents in the corpus.
     */
    long long getTotalTokens() const;
    
    // Persistence
    bool saveToDisk(const std::string& filepath) const;
    bool loadFromDisk(const std::string& filepath);
};
