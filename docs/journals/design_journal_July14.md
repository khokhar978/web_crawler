# Design Journal - July 14

## Goals for Today
1. Complete Project 2 by integrating all 5 crawler components into a main loop inside `main.cpp`.
2. Provide a stub for Project 3 (The Indexer) to verify that we can iterate through the SQLite database using strictly sequential IDs.

## What I Did
- I wrote a Breadth-First Search (BFS) crawler loop in `main.cpp`. It pulls a seed URL and max depth from command line arguments, pushes the seed to `URLFrontier`, and loops until the frontier is empty.
- Inside the loop, it checks `SeenURLStore` to avoid infinite loops, uses `HTTPClient` to download the HTML, `PageStorage` to save it to disk/SQLite, and `HTMLParser` to extract new links and push them to the frontier.
- I encountered a SQLite `UNIQUE constraint failed` crash when re-running the crawler because it attempted to insert URLs that were already in the database. I fixed this by modifying the `PageStorage` SQL query to use `INSERT OR IGNORE`.
- I discovered that using `INSERT OR REPLACE` breaks the strict sequential auto-incrementing IDs required by the instructor's Indexer loop, because SQLite deletes the old row and creates a new one at the end of the table. `INSERT OR IGNORE` was chosen because it safely leaves the old row intact.
- I debugged a `Timeout was reached` error with `libcurl` when downloading `iana.org`. I bypassed this by increasing the timeout to 30 seconds and setting a spoofed Chrome `CURLOPT_USERAGENT` to trick anti-bot protections.

## Outcomes
- The crawler is now fully operational! It successfully navigates links, archives data to a `.dat` file, and catalogs everything flawlessly in SQLite. 
- Project 2 is officially complete and ready for submission.
