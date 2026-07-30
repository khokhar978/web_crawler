# SuperCoders Web Crawler & Search Engine

A highly concurrent, C++ based web crawler and custom inverted index search engine designed specifically to index Wikipedia without triggering rate-limit bans. It uses the mathematical **Okapi BM25** ranking algorithm for enterprise-grade search relevance.

## Features

### 🕸️ Web Crawler
- **High Concurrency:** Utilizes a multi-threaded worker pool to crawl Wikipedia efficiently.
- **Firewall & Ban Protection:** Implements HTTP/1.1 Keep-Alive persistent connections (reusing libcurl handles) and honest Bot User-Agents (`SuperCodersBot/1.0`) to avoid triggering Cloudflare or Wikipedia DDoS protections.
- **Strict Validation:** Drops HTTP 429 and 404 error pages to ensure the dataset remains 100% pure HTML.
- **Data Persistence:** Backs up the Frontier queue (`frontier_backup.txt`) and stores massive binary HTML blobs via custom chunking in `crawler_archive.dat` backed by a SQLite3 metadata index.

### 🔍 Search Engine (Indexer)
- **Multi-Threaded Indexing:** Safely processes thousands of documents in parallel to build an Inverted Index using mutex locks and atomic operations.
- **True BM25 Ranking:** Replaces basic Term Frequency with the Okapi BM25 formula, intelligently normalizing scores based on document length relative to corpus average length to prevent keyword stuffing bias.
- **$O(N+M)$ Intersections:** Strictly sorts document IDs inside the postings list to allow lightning-fast linear pointer intersections for multi-word queries.
- **Interactive CLI:** Includes a built-in command line interface to query the dataset instantly with top-10 ranked results.

## Build Instructions

This project uses **CMake** and requires `libcurl` and `sqlite3`. 

```bash
# 1. Create a build directory
mkdir build
cd build

# 2. Generate Makefiles and Build
cmake ..
cmake --build .
```

This will produce two executables: `crawler.exe` and `indexer.exe`.

## Usage

### 1. Run the Crawler
Start the crawler by providing a seed URL and a max depth.
```bash
# Crawls starting from Computer Science up to 2 clicks deep
.\crawler.exe https://en.wikipedia.org/wiki/Computer_science 2
```
*Note: If the crawler is interrupted, running `.\crawler.exe` without arguments will automatically resume from the `frontier_backup.txt`.*

### 2. Build the Index & Search
Once you have crawled enough pages, run the indexer. It will automatically detect `crawler_archive.dat`, extract the HTML text, remove stop words, build the inverted index, and drop you into the search console.

```bash
.\indexer.exe
```

```
======================================
Welcome to SuperCoders Search!
Type a multi-word query to search (or 'exit' to quit).

Search> java programming
Found 460 results in 0.37 ms:
  1. [Score: 5.62] https://en.wikipedia.org/wiki/JVM
  2. [Score: 5.57] https://en.wikipedia.org/wiki/Netbeans
  3. [Score: 5.55] https://en.wikipedia.org/wiki/Java_Cryptography_Architecture
```
