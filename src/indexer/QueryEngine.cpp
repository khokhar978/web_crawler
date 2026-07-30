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
    
    // BM25 Hyperparameters
    const double k1 = 1.2;
    const double b = 0.75;
    
    // We use maxDocID as a proxy for total documents (N)
    int totalDocs = index.getTotalDocs();
    if (totalDocs < firstPostings.size()) totalDocs = firstPostings.size();
    
    // Calculate Average Document Length for BM25
    double avgDocLength = 1.0; // Prevent division by zero
    if (totalDocs > 0) {
        avgDocLength = static_cast<double>(index.getTotalTokens()) / totalDocs;
    }
    
    // Calculate IDF for the first token using BM25 IDF formula
    // IDF = ln( (N - n + 0.5) / (n + 0.5) + 1.0 )
    double n1 = static_cast<double>(firstPostings.size());
    double firstIDF = std::log((totalDocs - n1 + 0.5) / (n1 + 0.5) + 1.0);
    
    // Convert first postings into our working result set (which includes scores)
    DynamicArray<QueryResult> currentResults;
    for (int i = 0; i < firstPostings.size(); ++i) {
        int docID = firstPostings[i].docID;
        double tf = static_cast<double>(firstPostings[i].frequency);
        double docLength = static_cast<double>(index.getDocLength(docID));
        
        // True BM25 Term Frequency Saturation and Length Normalization
        double tfWeight = (tf * (k1 + 1.0)) / (tf + k1 * (1.0 - b + b * (docLength / avgDocLength)));
        double bm25Score = tfWeight * firstIDF;
        
        currentResults.append({docID, bm25Score});
    }
    
    // 3. Intersect with postings of all subsequent tokens (AND logic)
    for (int i = 1; i < tokens.size(); ++i) {
        DynamicArray<IndexPosting> nextPostings = index.search(tokens[i]);
        if (nextPostings.isEmpty()) {
            return DynamicArray<QueryResult>(); // AND logic fails
        }
        
        // Calculate BM25 IDF for this token
        double ni = static_cast<double>(nextPostings.size());
        double nextIDF = std::log((totalDocs - ni + 0.5) / (ni + 0.5) + 1.0);
        
        DynamicArray<QueryResult> intersected;
        
        // Both arrays are natively sorted by docID, allowing O(N+M) intersection
        int p1 = 0, p2 = 0;
        while (p1 < currentResults.size() && p2 < nextPostings.size()) {
            if (currentResults[p1].docID == nextPostings[p2].docID) {
                // Match found! Sum their BM25 scores
                int docID = currentResults[p1].docID;
                double tf = static_cast<double>(nextPostings[p2].frequency);
                double docLength = static_cast<double>(index.getDocLength(docID));
                
                double tfWeight = (tf * (k1 + 1.0)) / (tf + k1 * (1.0 - b + b * (docLength / avgDocLength)));
                double tokenBm25 = tfWeight * nextIDF;
                
                intersected.append({
                    docID, 
                    currentResults[p1].score + tokenBm25
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
