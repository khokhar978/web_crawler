#include <iostream>
#include <cassert>
#include "indexer/Tokenizer.h"

void testBasicTokenization() {
    std::string text = "Hello, World! This is a test.";
    DynamicArray<std::string> tokens = Tokenizer::tokenize(text);
    
    // Expected: hello, world, this, is, a, test
    assert(tokens.size() == 6);
    assert(tokens[0] == "hello");
    assert(tokens[1] == "world");
    assert(tokens[2] == "this");
    assert(tokens[3] == "is");
    assert(tokens[4] == "a");
    assert(tokens[5] == "test");
}

void testPunctuationAndCase() {
    std::string text = "DATA-structures... ARE  AWESOME!!!";
    DynamicArray<std::string> tokens = Tokenizer::tokenize(text);
    
    // Expected: data, structures, are, awesome
    assert(tokens.size() == 4);
    assert(tokens[0] == "data");
    assert(tokens[1] == "structures");
    assert(tokens[2] == "are");
    assert(tokens[3] == "awesome");
}

void testStopWordRemoval() {
    std::string text = "the quick brown fox jumps over the lazy dog and runs away";
    DynamicArray<std::string> tokens = Tokenizer::tokenize(text);
    
    Tokenizer::removeStopWords(tokens);
    
    // "the", "over", "the", "and" should be removed.
    // Remaining: quick, brown, fox, jumps, lazy, dog, runs, away
    assert(tokens.size() == 8);
    assert(tokens[0] == "quick");
    assert(tokens[1] == "brown");
    assert(tokens[2] == "fox");
    assert(tokens[3] == "jumps");
    assert(tokens[4] == "lazy");
    assert(tokens[5] == "dog");
    assert(tokens[6] == "runs");
    assert(tokens[7] == "away");
}

int main() {
    std::cout << "Running Tokenizer tests...\n";
    
    testBasicTokenization();
    std::cout << "[PASS] Basic Tokenization\n";
    
    testPunctuationAndCase();
    std::cout << "[PASS] Punctuation and Case normalization\n";
    
    testStopWordRemoval();
    std::cout << "[PASS] Stop Word Removal\n";
    
    std::cout << "All Tokenizer tests passed successfully!\n";
    return 0;
}
