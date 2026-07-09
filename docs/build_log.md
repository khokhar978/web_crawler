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
