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
