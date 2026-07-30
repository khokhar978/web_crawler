#pragma once
#include <string>
#include "indexer/InvertedIndex.h"
#include "common/DynamicArray.h"

// Represents a final search result ready to be displayed to the user
struct QueryResult {
    int docID;
    double score; // The relevance score (currently just the sum of Term Frequencies)
};

class QueryEngine {
public:
    /**
     * Executes a multi-word search against the Inverted Index.
     * Uses AND logic (document must contain ALL words).
     * Returns an array of QueryResults, sorted by score (highest first).
     */
    static DynamicArray<QueryResult> search(const std::string& query, const InvertedIndex& index);
    
private:
    /**
     * Helper method to sort results by score in descending order using Insertion Sort.
     */
    static void sortResults(DynamicArray<QueryResult>& results);
};
