# Design Journal - July 16

## Goals for Today
1. Improve the URL normalization logic to handle a wider variety of real-world link formats.
2. Address duplicate crawling issues when the crawler is restarted by implementing memory hydration.
3. Make the main crawling loop more interactive by prompting the user for new URLs instead of exiting when the frontier empties.
4. Prepare architectural groundwork for future Frontier disk persistence.

## What I Did
- I rewrote the URL resolution block in `HTMLParser.cpp`. The rudimentary resolver only handled absolute URLs and basic relative paths. The new logic robustly processes query strings (`?`), root-relative paths (`/`), protocol-relative links (`//`), and standard relative paths, matching true browser behavior.
- I identified a major flaw where restarting the crawler would wipe the in-memory `SeenStore`, causing it to re-download previously archived pages.
- I implemented a "Hydration" technique in `main.cpp`. On startup, the crawler queries the `PageStorage` SQLite database for all historically crawled URLs and pre-loads them into the in-memory `SeenStore`. This provides O(1) duplicate skipping across restarts without any SQL lookup penalties during the active crawl.
- I discovered an elegant edge case: we don't actually need to save the `SeenStore` to disk. Because the true state of "seen" URLs is just the union of (URLs in SQLite) + (URLs waiting in the Frontier queue), we can perfectly rebuild the `SeenStore` for free just by hydrating those two components on startup.
- I wrapped the main BFS crawler loop in `main.cpp` inside an outer `while(true)` loop. When the frontier runs dry, the crawler now pauses and prompts the user via `std::cin` for a new seed URL instead of immediately terminating.
- We analyzed the impact of our current architecture on a future distributed design. We confirmed that the modularity of `SeenURLStore` and `URLFrontier` will allow us to easily swap out the local HashMap/LinkedList for a centralized Redis/Kafka cluster in the future without changing the core crawling loop.

## Outcomes
- The crawler is now significantly more resilient to restarts. It skips duplicates flawlessly across multiple sessions.
- URL parsing is far more accurate and handles the vast majority of edge cases on the open web.
- The crawler operates continuously as a daemon-like process, waiting for user input when idle.
- The codebase is primed for the upcoming Frontier Persistence feature.
