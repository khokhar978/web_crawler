#include "indexer/InvertedIndex.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

InvertedIndex::InvertedIndex() : indexMap(1024), maxDocID(0), totalTokens(0) {
    // We initialize the HashMap with a capacity of 1024 to minimize
    // early rehashing overhead when indexing the first few documents.
}

void InvertedIndex::addDocument(int docID, const DynamicArray<std::string>& tokens) {
    // 1. Calculate raw frequencies WITHOUT holding the global lock
    HashMap<std::string, int> rawFreqs;
    for (int i = 0; i < tokens.size(); ++i) {
        if (!rawFreqs.contains(tokens[i])) {
            rawFreqs.put(tokens[i], 1);
        } else {
            rawFreqs.get(tokens[i])++;
        }
    }
    
    int docLength = tokens.size();
    DynamicArray<std::string> uniqueWords = rawFreqs.getKeys();
    
    // 2. Now acquire the lock to append data to the inverted index
    std::lock_guard<std::mutex> lock(indexMutex);
    
    if (docID > maxDocID) {
        maxDocID = docID;
    }
    
    // Expand docLengths array if necessary (since docIDs can have gaps)
    while (docLengths.size() <= docID) {
        docLengths.append(0);
    }
    docLengths[docID] = docLength;
    totalTokens += docLength;
    
    for (int i = 0; i < uniqueWords.size(); ++i) {
        const std::string& word = uniqueWords[i];
        int rawFreq = rawFreqs.get(word);
        
        DynamicArray<IndexPosting>& postings = indexMap[word];
        postings.append(IndexPosting(docID, rawFreq));
    }
}

int InvertedIndex::getDocLength(int docID) const {
    if (docID < docLengths.size()) {
        return docLengths[docID];
    }
    return 0;
}

long long InvertedIndex::getTotalTokens() const {
    return totalTokens;
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

void InvertedIndex::sortPostings() {
    std::lock_guard<std::mutex> lock(indexMutex);
    DynamicArray<std::string> keys = indexMap.getKeys();
    
    for (int i = 0; i < keys.size(); ++i) {
        DynamicArray<IndexPosting>& postings = indexMap[keys[i]];
        // Ensure standard sort is used to re-establish strictly ascending docID order
        std::sort(postings.begin(), postings.end());
    }
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
    
    // 2. Write the totalTokens
    out.write(reinterpret_cast<const char*>(&totalTokens), sizeof(long long));
    
    // 3. Write docLengths array
    int lengthsCount = docLengths.size();
    out.write(reinterpret_cast<const char*>(&lengthsCount), sizeof(int));
    for (int i = 0; i < lengthsCount; ++i) {
        int len = docLengths[i];
        out.write(reinterpret_cast<const char*>(&len), sizeof(int));
    }
    
    // 4. Write the number of keywords
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
    
    // 2. Read totalTokens
    if (!in.read(reinterpret_cast<char*>(&totalTokens), sizeof(long long))) {
        return false;
    }
    
    // 3. Read docLengths
    int lengthsCount = 0;
    if (!in.read(reinterpret_cast<char*>(&lengthsCount), sizeof(int))) {
        return false;
    }
    
    docLengths.clear();
    for (int i = 0; i < lengthsCount; ++i) {
        int len = 0;
        in.read(reinterpret_cast<char*>(&len), sizeof(int));
        docLengths.append(len);
    }
    
    // 4. Read total keywords
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
