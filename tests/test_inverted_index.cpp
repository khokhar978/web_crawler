#include <iostream>
#include <string>
#include <cassert>
#include "indexer/InvertedIndex.h"

void testBasicIndexingAndSearch() {
    InvertedIndex index;
    
    // Doc 1: "hello world"
    DynamicArray<std::string> doc1;
    doc1.append("hello");
    doc1.append("world");
    index.addDocument(1, doc1);
    
    // Doc 2: "world peace"
    DynamicArray<std::string> doc2;
    doc2.append("world");
    doc2.append("peace");
    index.addDocument(2, doc2);
    
    // Search "hello" -> should return [{1, 1}]
    DynamicArray<IndexPosting> resHello = index.search("hello");
    assert(resHello.size() == 1);
    assert(resHello[0].docID == 1);
    assert(resHello[0].frequency == 1);
    
    // Search "world" -> should return [{1, 1}, {2, 1}]
    DynamicArray<IndexPosting> resWorld = index.search("world");
    assert(resWorld.size() == 2);
    assert(resWorld[0].docID == 1);
    assert(resWorld[0].frequency == 1);
    assert(resWorld[1].docID == 2);
    assert(resWorld[1].frequency == 1);
    
    // Search "peace" -> should return [{2, 1}]
    DynamicArray<IndexPosting> resPeace = index.search("peace");
    assert(resPeace.size() == 1);
    assert(resPeace[0].docID == 2);
    assert(resPeace[0].frequency == 1);
    
    // Search non-existent -> should return empty array
    DynamicArray<IndexPosting> resNone = index.search("alien");
    assert(resNone.size() == 0);
}

void testFrequencyCounting() {
    InvertedIndex index;
    
    // Doc 1: "apple apple apple"
    DynamicArray<std::string> doc1;
    doc1.append("apple");
    doc1.append("apple");
    doc1.append("apple");
    index.addDocument(1, doc1);
    
    // Search "apple" -> should return [{1, 3}] exactly once
    DynamicArray<IndexPosting> resApple = index.search("apple");
    assert(resApple.size() == 1);
    assert(resApple[0].docID == 1);
    assert(resApple[0].frequency == 3);
}

int main() {
    std::cout << "Running InvertedIndex tests with Frequencies...\n";
    
    testBasicIndexingAndSearch();
    std::cout << "[PASS] Basic Indexing and Search\n";
    
    testFrequencyCounting();
    std::cout << "[PASS] Frequency Counting\n";
    
    std::cout << "All InvertedIndex tests passed successfully!\n";
    return 0;
}
