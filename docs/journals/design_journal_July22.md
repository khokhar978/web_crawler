# Design Journal - July 22, 2026

## Overview of Architectural Changes & Improvements

### 1. Database Hydration & Batch Querying (`PageStorage` & `main.cpp`)
- **Problem**: When restarting the crawler, checking URLs individually against the database or missing previously stored URLs can lead to duplicate crawls and overhead.
- **Solution**: We implemented a new hydration logic inside `main.cpp`. On startup, it retrieves all previously crawled URLs in a single batch query.
- **Implementation**: Added `getAllSeenURLs()` to `PageStorage.cpp` which executes a single, highly performant `SELECT url FROM crawler_metadata;` statement instead of multiple queries. This populates our `SeenStore` quickly.

### 2. Custom Data Structure Consistency (`URLFrontier`)
- **Problem**: `URLFrontier::getQueuedUrls()` was returning a standard `std::vector<std::string>`, breaking the pattern of utilizing our custom STL-like components throughout the project.
- **Solution**: Completely replaced `std::vector` with our custom `DynamicArray`.
- **Implementation**: Modified `URLFrontier.h` and `URLFrontier.cpp` to initialize a `DynamicArray` with the exact required capacity (based on `queue.getSize()`) and append elements to it.

### 3. Robust HTML Link Extraction (`HTMLParser`)
- **Problem**: The existing HTML parser was extremely brittle. It relied on exact string matches for `"<a "` and `"href=\""`, which caused it to skip tags using single quotes, uppercase tags (e.g. `<A HREF=`), and tags with unconventional whitespace.
- **Solution**: Built a highly efficient, character-by-character scanner to parse anchor tags natively.
- **Implementation**: 
  - Uses `std::tolower` to ensure case-insensitivity.
  - Safely handles whitespace checking (tabs, newlines, multiple spaces) before attempting to find the `href`.
  - Searches for `h r e f =` case-insensitively and gracefully steps through spaces until it hits either a `'` or `"`.
  - Parses the URL bounds appropriately using the matching quote character.
