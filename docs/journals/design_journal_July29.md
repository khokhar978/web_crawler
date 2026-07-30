# Design Journal - July 29, 2026

## Overview of Architectural Changes & Improvements

### 1. Persistent Binary Indexing
- **Problem**: Building the Inverted Index for 11,812 Wikipedia pages took almost 6 minutes on startup. This made testing the search query engine extremely tedious, as we had to rebuild the massive memory map every single time.
- **Solution**: Implemented binary serialization to dump the raw memory of the `InvertedIndex` directly to a file (`inverted_index.dat`) for O(1) loading.
- **Implementation**: 
  - Created `saveToDisk` and `loadFromDisk` methods in `InvertedIndex`.
  - Serialized the layout exactly as `Total Keywords -> [Keyword Length + Keyword String + Total Postings -> [DocID, TF, DocID, TF...]]`.
  - Bypassed all string parsing on load. Resulted in dropping the load time from 346 seconds down to just 1.6 seconds.

### 2. Disk I/O Batching & Syscall Elimination
- **Problem**: During indexing, our Disk Active Time wasn't peeking and the CPU was stalling for entire seconds. Profiling showed that the `PageStorage` was doing 1,600 SQLite queries (one for `getURLByID` and one for `getPage`) and 800 OS `open()` and `close()` file stream calls per 800-page chunk.
- **Solution**: Shifted the chunking logic straight into the database layer via `getPageBatch`.
- **Implementation**: 
  - Executed a single SQLite query `SELECT id, offset, length ... LIMIT 800` to fetch all metadata instantly.
  - Opened `crawler_archive.dat` exactly once per batch, ripped through all 800 byte offsets into a `DynamicArray<PageData>`, and closed the file.
  - Eliminated the OS Kernel Syscall overhead entirely.

### 3. Producer-Consumer Double Buffering Pipeline
- **Problem**: Even with batching, the CPU threads had to wait for a fraction of a second while the main thread read the next chunk from disk, resulting in visible 30% dips on the Task Manager graph.
- **Solution**: Architected a true Double-Buffering Producer-Consumer pipeline.
- **Implementation**: 
  - Spawned a 9th dedicated `diskThread` that constantly reads the *next* 800 pages in the background.
  - While the 8 CPU workers are processing the current batch, the 9th thread gets the next batch ready in RAM.
  - When the workers finish, they seamlessly swap the data arrays, mathematically erasing disk read latency.

### 4. Dynamic Atomic Load Balancing
- **Problem**: The CPU graph still exhibited small dips. We identified this as Thread Imbalance (Straggler Threads). Because Wikipedia pages vary wildly in size (from 1MB to 10KB), static partitioning caused some threads to finish early and go idle while others were stuck on massive articles.
- **Solution**: Replaced static array slicing with Dynamic Load Balancing using an atomic lock-free queue.
- **Implementation**: 
  - Created `std::atomic<int> next_page_idx`.
  - All 8 threads run a while-loop doing `next_page_idx.fetch_add(1)` to instantly claim the very next available page the millisecond they finish parsing their current one.
  - Guaranteeing zero idle threads, flattening the CPU graph to a continuous 100% block, and setting a record index time of 138.4 seconds for nearly 2GB of HTML.
