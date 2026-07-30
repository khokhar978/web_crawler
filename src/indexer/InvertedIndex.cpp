#include "indexer/InvertedIndex.h"
#include <fstream>
#include <iostream>

InvertedIndex::InvertedIndex() : indexMap(1024), maxDocID(0) {
    // We initialize the HashMap with a capacity of 1024 to minimize
    // early rehashing overhead when indexing the first few documents.
}

void InvertedIndex::addDocument(int docID, const DynamicArray<std::string>& tokens) {
    std::lock_guard<std::mutex> lock(indexMutex);
    
    if (docID > maxDocID) {
        maxDocID = docID;
    }
    
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

int InvertedIndex::getTotalDocs() const {
    return maxDocID;
}

bool InvertedIndex::saveToDisk(const std::string& filepath) const {
    std::lock_guard<std::mutex> lock(indexMutex);
    
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open index file for writing: " << filepath << std::endl;
        return false;
    }
    
    DynamicArray<std::string> keys = indexMap.getKeys();
    int totalKeys = keys.size();
    
    // 1. Write the maxDocID
    out.write(reinterpret_cast<const char*>(&maxDocID), sizeof(int));
    
    // 2. Write the number of keywords
    out.write(reinterpret_cast<const char*>(&totalKeys), sizeof(int));
    
    for (int i = 0; i < totalKeys; ++i) {
        const std::string& key = keys[i];
        
        // Write the length of the string, then the string itself
        int keyLen = key.length();
        out.write(reinterpret_cast<const char*>(&keyLen), sizeof(int));
        out.write(key.c_str(), keyLen);
        
        // Write the number of postings for this keyword
        const DynamicArray<IndexPosting>& postings = indexMap.get(key);
        int postingsCount = postings.size();
        out.write(reinterpret_cast<const char*>(&postingsCount), sizeof(int));
        
        // Write all postings directly from memory (they are POD structs)
        for (int p = 0; p < postingsCount; ++p) {
            const IndexPosting& posting = postings[p];
            out.write(reinterpret_cast<const char*>(&posting), sizeof(IndexPosting));
        }
    }
    
    out.close();
    return true;
}

bool InvertedIndex::loadFromDisk(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(indexMutex);
    
    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) {
        // File doesn't exist yet, which is fine on first run
        return false;
    }
    
    indexMap.clear();
    
    // 1. Read maxDocID
    if (!in.read(reinterpret_cast<char*>(&maxDocID), sizeof(int))) {
        return false;
    }
    
    // 2. Read total keywords
    int totalKeys = 0;
    if (!in.read(reinterpret_cast<char*>(&totalKeys), sizeof(int))) {
        return false;
    }
    
    for (int i = 0; i < totalKeys; ++i) {
        int keyLen = 0;
        in.read(reinterpret_cast<char*>(&keyLen), sizeof(int));
        
        std::string key;
        key.resize(keyLen);
        in.read(&key[0], keyLen);
        
        int postingsCount = 0;
        in.read(reinterpret_cast<char*>(&postingsCount), sizeof(int));
        
        // Prepare the postings array
        DynamicArray<IndexPosting>& postings = indexMap[key];
        
        for (int p = 0; p < postingsCount; ++p) {
            IndexPosting posting;
            in.read(reinterpret_cast<char*>(&posting), sizeof(IndexPosting));
            postings.append(posting);
        }
    }
    
    in.close();
    return true;
}
