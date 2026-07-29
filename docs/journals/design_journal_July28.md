# Design Journal - July 28, 2026

## Overview of Architectural Changes & Improvements

### 1. Core Indexer Components (`HTMLTextExtractor`, `Tokenizer`, `InvertedIndex`)
- **Problem**: Our crawler successfully stores vast amounts of raw HTML, but it's not searchable. We need to process this HTML into an efficient format mapping keywords back to their source URLs with frequency counts.
- **Solution**: Designed the three core pillars of the Search Engine Indexer.
- **Implementation**: 
  - `HTMLTextExtractor`: Built an O(N) state-machine based parser that strips all HTML tags, styles, and scripts to extract raw readable text.
  - `Tokenizer`: Cleans the text by lowercasing, removing punctuation, and filtering out useless stop words (like "the", "and") in O(1) time using our custom `HashMap` preloaded with a static dictionary.
  - `InvertedIndex`: Uses our `HashMap` to map each word to a `DynamicArray` of `IndexPosting` structs. Instead of just storing Document IDs, the struct inherently calculates and stores the Term Frequency (TF) of the word inside each document for future ranking.

### 2. Main Indexer Pipeline & CMake Restructuring
- **Problem**: We needed an executable to seamlessly bridge the Crawler's database output to our Indexer logic, but simply adding `indexer/main.cpp` caused linker errors due to multiple `main()` definitions with the crawler.
- **Solution**: Refactored `CMakeLists.txt` to cleanly isolate module sources and built the Indexer pipeline loop.
- **Implementation**: 
  - Separated the CMake `GLOB_RECURSE` into specific lists (`COMMON_SOURCES`, `CRAWLER_SOURCES`, `INDEXER_SOURCES`).
  - Generated two entirely separate and clean executables: `crawler.exe` and `indexer.exe`.
  - The Indexer main loop fetches pages sequentially via `PageStorage`, pipelines them through the extractor and tokenizer, and feeds them into the `InvertedIndex`.

### 3. Crawler Optimizations & Strict Domain Filtering
- **Problem**: When tested on Wikipedia, the crawler queue exploded to over 250,000 URLs at Depth 2. This was because Wikipedia sidebars link to 200+ foreign languages (`fr.wikipedia.org`, etc.), and our naive `"wikipedia.org"` check allowed all of them. Also, resuming the crawler from a massive backup loaded thousands of URLs that took forever to discard.
- **Solution**: Implemented memory-efficient domain filtering at the parser level and an early-exit check in the crawler loop.
- **Implementation**: 
  - Refactored `HTMLParser::extractLinks` to accept an optional `allowedDomain` parameter. This discards invalid URLs before they are even added to the returned array, saving immense memory.
  - Locked the crawler strictly to `"en.wikipedia.org"`.
  - Added an early-exit check (`if (currentDepth > maxDepth) continue;`) to instantly drop URLs exceeding the requested max depth upon resumption.
  - Downsampled the massive 250k URL frontier queue to exactly 15,000 URLs via a Python script to ensure a fast, targeted English crawl.
