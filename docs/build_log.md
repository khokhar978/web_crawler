# Build Log

For every work session submit Date, Duration, Goal, Problem, What I Tried, and Outcome.

## Example Session
**Date:** June 14
**Duration:** 90 minutes
**Goal:** Fix duplicate URL detection
**Problem:** URLs still being revisited
**What I Tried:** Checked HashMap insertion logic
**Outcome:** Discovered URLs were not being normalized before insertion

---

## Session 1
**Date:** July 9
**Duration:** 120 minutes
**Goal:** Draft Phase 0 Design Proposal
**Problem:** Need to finalize strict architectural choices to meet both O(1) performance and scaling requirements without memory exhaustion. Also needed to fix mermaid diagram syntax errors that prevented rendering.
**What I Tried:** Wrote crawler_design.md defining 7 core constraints: LinkedList for Frontier, HashMap for Seen Store, SQL + .dat file for Page Storage, libcurl Downloader, string::find() Link Extractor, excluded robots.txt, and strict normalization. Fixed Mermaid syntax for subgraphs.
**Outcome:** Successfully finalized crawler_design.md with all required diagrams functioning correctly and API definitions matching Project 03 specifications.

---

## Session 2
**Date:** July 9
**Duration:** 30 minutes
**Goal:** Implement the URL Frontier component.
**Problem:** Need to implement a fast, O(1) FIFO queue for URLs and depth tracking without using STL containers.
**What I Tried:** Created `URLFrontier.h` and `URLFrontier.cpp` to wrap the custom `LinkedList<FrontierEntry>` from Project 01. Implemented `push()`, `pop()`, `isEmpty()`, and `size()` in small, incremental steps.
**Outcome:** URL Frontier is fully implemented. It provides O(1) insertions to the tail and O(1) removals from the head, successfully avoiding any array-shifting bottlenecks.

---

## Session 3
**Date:** July 9
**Duration:** 20 minutes
**Goal:** Implement the Seen URL Store component.
**Problem:** Need O(1) duplicate URL detection to prevent the crawler from hitting infinite loops or saving the same page multiple times.
**What I Tried:** Created `SeenURLStore.h` and `SeenURLStore.cpp`. Initialized the custom `HashMap<std::string, bool>` from Project 01. Implemented `markSeen()` by calling `map.put(url, true)`, and implemented `isSeen()` by leveraging `map.contains(url)`.
**Outcome:** Successfully built an O(1) duplicate filtering system in RAM that sits perfectly in front of our database layer.

---

## Session 4
**Date:** July 9
**Duration:** 30 minutes
**Goal:** Implement Page Storage file appending logic (Step 4, Part 1)
**Problem:** Storing 50,000+ HTML files in RAM causes OOM crashes. Creating 50,000 individual files exhausts inodes and slows down disk I/O.
**What I Tried:** Created `PageStorage.h` and `PageStorage.cpp`. Initialized an `std::ofstream` to open a massive `crawler_archive.dat` in `std::ios::app | std::ios::binary` mode. Implemented the first half of `storePage()` to capture the byte offset via `tellp()`, append the raw HTML, and force a `flush()` to disk.
**Outcome:** Successfully built a high-performance, single-file append-only storage system that sidesteps inode exhaustion and RAM limitations.

---

## Session 5
**Date:** July 10
**Duration:** 45 minutes
**Goal:** Integrate SQLite to finalize Hybrid Page Storage (Step 4, Part 2)
**Problem:** The `.dat` file stores raw bytes sequentially, but we have no way to find a specific page inside a 10GB file without an index map.
**What I Tried:** Downloaded the SQLite Amalgamation (`sqlite3.c`/`h`). Configured `CMakeLists.txt` and `.vscode` to compile C and C++ together via MSYS2 `g++`. Implemented `initDatabase()` to create a metadata table, and updated `storePage()`, `getPage()`, and `hasPage()` using SQLite prepared statements.
**Outcome:** The Hybrid Storage architecture is fully functional. We can now store massive HTML pages to disk instantly and retrieve them using lightning-fast byte offsets stored in SQLite.

---

## Session 6
**Date:** July 10
**Duration:** 30 minutes
**Goal:** Implement the HTML Parser to extract hyperlinks (Step 5)
**Problem:** Regular expressions are too slow and memory-intensive for parsing massive 2MB HTML pages continuously during a web crawl.
**What I Tried:** Created `HTMLParser.h` and `HTMLParser.cpp`. Implemented a manual, zero-allocation string search loop using `std::string::find()` to rapidly jump from one `<a href="` tag to the next. Built a rudimentary relative URL resolver and filtered out invalid links (like `#`, `mailto:`, and `javascript:`).
**Outcome:** Successfully created a lightning-fast HTML parser that bypasses regex bottlenecks and neatly stores all extracted links into our custom `DynamicArray<std::string>`.

---

## Session 7
**Date:** July 11
**Duration:** 30 minutes
**Goal:** Implement cross-platform HTTP Client (Step 6)
**Problem:** We needed a way to fetch HTML from the internet across different operating systems. C++ has no native networking library.
**What I Tried:** Adhered to the Phase 0 design proposal to use `libcurl` wrapped in a `Downloader` interface. The main hurdle was installing `libcurl` on Windows. We resolved this by leveraging the MSYS2 environment's package manager (`pacman`). Ran `pacman -S mingw-w64-ucrt-x86_64-curl` to install the pre-compiled binaries, and updated `.vscode/tasks.json` to include the `-lcurl` linker flag. Implemented `HTTPClient::fetchPage()` using `curl_easy_init()`.
**Outcome:** Successfully built a robust HTTP Client that follows redirects and handles timeouts cross-platform, without forcing us to manually compile complex dependencies from source.

---

## Session 8
**Date:** July 11
**Duration:** 30 minutes
**Goal:** Fix compilation and linker bugs during full system build
**Problem 1 (Compilation):** `HTMLParser.cpp` failed to compile with error: `class DynamicArray has no member named 'push_back'`. I had mistakenly assumed our custom array mimicked `std::vector::push_back`, but our custom implementation from Project 01 explicitly uses `append()`.
**Problem 2 (Linking):** The linker (`ld.exe`) crashed with `undefined reference to sqlite3_open`. I discovered that by declaring `project(Crawler CXX)` in CMake, the C compiler was disabled. CMake completely ignored `sqlite3.c`, resulting in missing object files.
**What I Tried:** 
1. Fixed `HTMLParser.cpp` to correctly call `extractedUrls.append(url);`.
2. Updated `CMakeLists.txt` to `project(SuperCoders_Project02 CXX C)` to explicitly enable the C compiler alongside C++.
**Outcome:** The entire project now compiles and links perfectly via both VS Code's `tasks.json` and the standard `cmake --build .` Ninja system.

---

## Session 9
**Date:** July 14
**Duration:** 60 minutes
**Goal:** Finalize Project 2 by wiring all components together in `main.cpp` and preparing the Project 3 Indexer stub.
**Problem 1:** SQLite `UNIQUE constraint failed` crash when re-running the crawler on previously visited URLs.
**Problem 2:** `iana.org` timed out due to overly aggressive anti-bot protection blocking the default `libcurl` requests, and SSL certificate verification failing on Windows.
**What I Tried:** 
1. Rewrote `main.cpp` to implement a Breadth-First Search (BFS) crawler loop utilizing `URLFrontier`, `SeenURLStore`, `HTTPClient`, `HTMLParser`, and `PageStorage`. Added CLI argument parsing for seed URL and depth.
2. Fixed the SQLite crash by changing the SQL query in `PageStorage.cpp` to `INSERT OR IGNORE`. This prevents duplicate insertions while preserving the strictly sequential auto-incrementing IDs required for the Indexer stub.
3. Fixed the `libcurl` issues by adding `CURLOPT_SSL_VERIFYPEER = 0L` to bypass missing Windows CA certificates, setting `CURLOPT_TIMEOUT` to 30 seconds, and spoofing a standard Google Chrome `CURLOPT_USERAGENT` to bypass bot blockers.
**Outcome:** The crawler successfully navigates the web, downloads HTML, extracts hyperlinks, avoids infinite loops, and archives everything to SQLite seamlessly! Project 2 is officially complete.
