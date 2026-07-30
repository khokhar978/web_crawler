#include "indexer/QueryEngine.h"
#include "indexer/Tokenizer.h"
#include <cmath>

void QueryEngine::sortResults(DynamicArray<QueryResult>& results) {
    // Simple Insertion Sort to order results by score (descending)
    // Suitable since search result sets for an AND query are typically small
    for (int i = 1; i < results.size(); i++) {
        QueryResult key = results[i];
        int j = i - 1;
        
        while (j >= 0 && results[j].score < key.score) {
            results[j + 1] = results[j];
            j = j - 1;
        }
        results[j + 1] = key;
    }
}

DynamicArray<QueryResult> QueryEngine::search(const std::string& query, const InvertedIndex& index) {
    // 1. Tokenize query and remove stop words using our Tokenizer module
    DynamicArray<std::string> tokens = Tokenizer::tokenize(query);
    Tokenizer::removeStopWords(tokens);
    
    // If the query was empty or only contained stop words
    if (tokens.isEmpty()) {
        return DynamicArray<QueryResult>();
    }
    
    // 2. Get initial postings for the first valid token
    DynamicArray<IndexPosting> firstPostings = index.search(tokens[0]);
    if (firstPostings.isEmpty()) {
        return DynamicArray<QueryResult>(); // AND logic fails instantly if any word is missing
    }
    
    // We use maxDocID as a proxy for total documents (N)
    int totalDocs = index.getTotalDocs();
    if (totalDocs < firstPostings.size()) totalDocs = firstPostings.size();
    
    // Calculate IDF for the first token
    double firstIDF = std::log(static_cast<double>(totalDocs) / firstPostings.size());
    
    // Convert first postings into our working result set (which includes scores)
    DynamicArray<QueryResult> currentResults;
    for (int i = 0; i < firstPostings.size(); ++i) {
        // Sublinear TF Scaling: 1 + log(TF) to prevent word repetition spam
        double tfWeight = 1.0 + std::log(static_cast<double>(firstPostings[i].frequency));
        // TF-IDF = TF_weight * IDF
        double tfIdfScore = tfWeight * firstIDF;
        currentResults.append({firstPostings[i].docID, tfIdfScore});
    }
    
    // 3. Intersect with postings of all subsequent tokens (AND logic)
    for (int i = 1; i < tokens.size(); ++i) {
        DynamicArray<IndexPosting> nextPostings = index.search(tokens[i]);
        if (nextPostings.isEmpty()) {
            return DynamicArray<QueryResult>(); // AND logic fails
        }
        
        // Calculate IDF for this token
        double nextIDF = std::log(static_cast<double>(totalDocs) / nextPostings.size());
        
        DynamicArray<QueryResult> intersected;
        
        // Both arrays are natively sorted by docID, allowing O(N+M) intersection
        int p1 = 0, p2 = 0;
        while (p1 < currentResults.size() && p2 < nextPostings.size()) {
            if (currentResults[p1].docID == nextPostings[p2].docID) {
                // Match found! Sum their TF-IDF scores
                // Sublinear TF Scaling: 1 + log(TF)
                double tfWeight = 1.0 + std::log(static_cast<double>(nextPostings[p2].frequency));
                double tokenTfIdf = tfWeight * nextIDF;
                
                intersected.append({
                    currentResults[p1].docID, 
                    currentResults[p1].score + tokenTfIdf
                });
                p1++;
                p2++;
            } else if (currentResults[p1].docID < nextPostings[p2].docID) {
                p1++;
            } else {
                p2++;
            }
        }
        currentResults = intersected; // Overwrite with the newly intersected list
        
        if (currentResults.isEmpty()) {
            break; // No overlapping documents remain, exit early
        }
    }
    
    // 4. Sort the final intersected list so the highest scoring documents are first
    sortResults(currentResults);
    
    return currentResults;
}
