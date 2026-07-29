#include <iostream>
#include <string>
#include <cassert>
#include "indexer/InvertedIndex.h"
#include "indexer/QueryEngine.h"

void testIntersectionAndRanking() {
    InvertedIndex index;
    
    // Doc 1: "machine learning is fun machine"
    DynamicArray<std::string> doc1;
    doc1.append("machine");
    doc1.append("learning");
    doc1.append("is");
    doc1.append("fun");
    doc1.append("machine");
    index.addDocument(1, doc1);
    
    // Doc 2: "machine language learning"
    DynamicArray<std::string> doc2;
    doc2.append("machine");
    doc2.append("language");
    doc2.append("learning");
    index.addDocument(2, doc2);
    
    // Doc 3: "learning to fly"
    DynamicArray<std::string> doc3;
    doc3.append("learning");
    doc3.append("to");
    doc3.append("fly");
    index.addDocument(3, doc3);
    
    // Search 1: "machine learning"
    // Expect: Doc 1 (Score 3: 2 machine + 1 learning)
    //         Doc 2 (Score 2: 1 machine + 1 learning)
    // Doc 3 doesn't have "machine", so it should be excluded (AND logic).
    DynamicArray<QueryResult> results = QueryEngine::search("machine learning", index);
    
    assert(results.size() == 2);
    assert(results[0].docID == 1); // Doc 1 has higher score
    assert(results[0].score == 3);
    assert(results[1].docID == 2); // Doc 2 has lower score
    assert(results[1].score == 2);
    
    // Search 2: "language"
    // Expect: Doc 2 (Score 1)
    DynamicArray<QueryResult> resLang = QueryEngine::search("language", index);
    assert(resLang.size() == 1);
    assert(resLang[0].docID == 2);
    assert(resLang[0].score == 1);
    
    // Search 3: "fun machine fly"
    // Expect: Empty (no document has all three)
    DynamicArray<QueryResult> resEmpty = QueryEngine::search("fun machine fly", index);
    assert(resEmpty.size() == 0);
}

int main() {
    std::cout << "Running QueryEngine tests...\n";
    
    testIntersectionAndRanking();
    std::cout << "[PASS] Intersection and Ranking\n";
    
    std::cout << "All QueryEngine tests passed successfully!\n";
    return 0;
}
