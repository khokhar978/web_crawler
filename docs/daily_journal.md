# Daily Journal - Search Engine & Crawler Optimization

## Date: July 30, 2026

### 1. BM25 Ranking Engine Optimization
- Identified an issue where the multi-threaded indexer was generating unsynchronized `docIDs`, causing the $O(N+M)$ search intersection loop to drop valid pages.
- **Fix:** Implemented `InvertedIndex::sortPostings` using `std::sort` to forcefully synchronize all `docIDs` across all keyword maps before saving the inverted index to disk.
- Replaced the basic Term Frequency ranking with the mathematically sound **Okapi BM25** algorithm.
- Updated `PageStorage` and `InvertedIndex` binary formats to store and read total corpus tokens and individual document token lengths (`docLengths`). This allows the BM25 formula to dynamically calculate `avgDocLength` and prevent massive Wikipedia pages from inherently dominating the search results.

### 2. Crawler Networking & Anti-Ban Upgrades
- Investigated why the crawler was throwing errors and found Wikipedia's Web Application Firewall (WAF) was blocking us via HTTP 429 (Too Many Requests) due to hitting 166 requests/sec.
- **Status Code Validation:** The crawler previously failed to check HTTP response codes, saving 652KB Cloudflare CAPTCHA and 429 Error pages into `crawler_archive.dat`. Added `curl_easy_getinfo(..., CURLINFO_RESPONSE_CODE)` to automatically discard any non-200 OK responses.
- **HTTP/1.1 Keep-Alive:** Traced a major performance and ban issue to `HTTPClient` creating a brand new TLS/TCP connection for every single page. Refactored the architecture to instantiate `HTTPClient` outside the worker loop, persisting the `CURL*` handle for the entire thread's lifespan.
- **Honest User-Agent:** Changed the generic browser User-Agent to `SuperCodersBot/1.0 (contact@example.com)` to comply with Wikipedia's bot policies and bypass JavaScript execution challenges.
- **Rate-Limiting:** Capped the crawler thread pool to 8 workers and added a `200ms` `sleep_for` delay per thread to guarantee compliance with Wikipedia's scraping limits.

### 3. Execution & Verification
- Cleaned out the corrupted 9GB database and archived files.
- Shrunk the 350,000 URL frontier queue down to 5,000 URLs for a focused test crawl.
- Successfully crawled, indexed, and queried terms like `java`, yielding perfect results (e.g., JVM, Netbeans, Sun Microsystems).
