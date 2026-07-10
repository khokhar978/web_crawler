# Weekly Design Journal

Submit weekly. Every entry must contain all five sections.

## Entry July 9 (Phase 0 Design)

**Section 1 — Specific Bug:**
During the initial crawler design phase, we realized that removing a URL from the front of the frontier queue using a standard dynamic array would cause an $O(N)$ memory shift bottleneck, slowing down the crawler exponentially as the queue grew to millions of URLs.

**Section 2 — Failed Attempt:**
I originally considered using `DynamicArray<FrontierEntry>` for the queue. However, every time `pop()` is called to get the next URL to crawl, all remaining elements in the array would need to shift left by one memory address, which is catastrophic for performance.

**Section 3 — Memory Diagram:**
```mermaid
graph TD
    subgraph O1_Queue_Memory_Model [LinkedList O(1) Pop]
        direction LR
        head["Head Pointer"] --> Node1["Node 1 (To be Popped)"]
        Node1 --> Node2["Node 2 (New Head)"]
        Node2 --> Node3["Node 3"]
    end
```
*By using a LinkedList, popping just deletes Node 1 and moves the Head Pointer to Node 2 without shifting any memory.*

**Section 4 — Code Reference:**
Commit: "Draft Phase 0 Design Proposal"
File: `docs/crawler_design.md` & `include/URLFrontier.h`

**Section 5 — Learning Reflection:**
I learned that choosing the right data structure is critical for systems programming. While arrays provide cache-locality, a LinkedList is the absolute correct choice for a First-In-First-Out (FIFO) queue where $O(1)$ insertions at the tail and $O(1)$ removals at the head are strictly required.
