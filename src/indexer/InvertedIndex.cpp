#include "indexer/InvertedIndex.h"

InvertedIndex::InvertedIndex() : indexMap(1024) {
    // We initialize the HashMap with a capacity of 1024 to minimize
    // early rehashing overhead when indexing the first few documents.
}

void InvertedIndex::addDocument(int docID, const DynamicArray<std::string>& tokens) {
    for (int i = 0; i < tokens.size(); ++i) {
        const std::string& word = tokens[i];
        
        // HashMap::operator[] retrieves the value if it exists, or inserts 
        // a default-constructed DynamicArray<IndexPosting> and returns a reference to it.
        DynamicArray<IndexPosting>& postings = indexMap[word];
        
        // Since we process one document at a time sequentially, all docIDs 
        // appended to a word's list are naturally sorted.
        // We check the last added posting to see if we've already recorded this docID.
        if (postings.isEmpty() || postings[postings.size() - 1].docID != docID) {
            // First time seeing this word in this document
            postings.append(IndexPosting(docID, 1));
        } else {
            // We've seen this word in this document before, just increment the frequency
            postings[postings.size() - 1].frequency++;
        }
    }
}

DynamicArray<IndexPosting> InvertedIndex::search(const std::string& query) const {
    if (indexMap.contains(query)) {
        // This will invoke the DynamicArray copy constructor, creating 
        // a safe, decoupled copy for the caller.
        return indexMap.get(query);
    }
    
    // Return an empty array if the keyword is not found anywhere
    return DynamicArray<IndexPosting>();
}

int InvertedIndex::size() const {
    return indexMap.size();
}
