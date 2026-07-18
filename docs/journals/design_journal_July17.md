# Design Journal - July 17

## Goals for Today
1. Implement Frontier Disk Persistence so the crawler can resume from exactly where it left off after a crash.
2. Replace all raw `std::cout` logging with a professional, timestamped Logger class.
3. Add `robots.txt` compliance so the crawler respects website crawling policies.

## What I Did
- Implemented `saveToFile()` and `loadFromFile()` in `URLFrontier` to serialize the `LinkedList` queue to a `frontier_backup.txt` file and restore it on startup.
- Discovered a critical performance issue: overwriting the entire backup file on every single URL would create a massive disk I/O bottleneck at scale (100,000+ URLs in queue). Solved this by introducing a `pagesCrawled` counter and only saving every 50 pages. The SQLite database acts as ground truth for already-crawled pages, so the worst case is re-checking a handful of stale URLs on restart.
- Found and fixed a subtle bug where hydrating the `SeenStore` from the backup file caused the crawler to think all 6682 queued URLs were already crawled, instantly draining the entire queue. Removed the faulty hydration logic — the pop() loop's own `isSeen()` check already handles this correctly.
- Made command-line arguments fully optional. The crawler can now boot with zero arguments and resume entirely from its saved state.
- Created `Logger.h` and `Logger.cpp` with `INFO`, `WARN`, `ERROR`, and `DEBUG` severity levels. Every message is timestamped and dual-routed to both the console and a persistent `crawler.log` file.
- Built `RobotsChecker.h` and `RobotsChecker.cpp` with a per-domain caching system using our custom `HashMap`. Each domain's `robots.txt` is fetched exactly once, parsed for `Disallow:` rules, and cached for all subsequent URL checks on that domain.

## Outcomes
- The crawler is now fully crash-resilient — it saves its queue periodically and resumes flawlessly on restart.
- All output is professional, timestamped, and permanently recorded in `crawler.log`.
- The crawler ethically respects `robots.txt` rules with zero performance penalty thanks to the per-domain cache.
