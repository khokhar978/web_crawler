#ifndef CRAWL_STATS_H
#define CRAWL_STATS_H

#include <string>
#include <ctime>
#include "Logger.h"

// Tracks crawl metrics throughout a session and prints a formatted summary
struct CrawlStats {
    int pagesDownloaded = 0;
    int failedDownloads = 0;
    int robotsBlocked = 0;
    int linksExtracted = 0;
    int duplicatesSkipped = 0;
    long long totalBytesDownloaded = 0;
    std::time_t startTime = 0;

    void start() {
        startTime = std::time(nullptr);
    }

    void recordDownload(int bytes) {
        pagesDownloaded++;
        totalBytesDownloaded += bytes;
    }

    void recordFailure() {
        failedDownloads++;
    }

    void recordRobotsBlock() {
        robotsBlocked++;
    }

    void recordLinksExtracted(int count) {
        linksExtracted += count;
    }

    void recordDuplicateSkip() {
        duplicatesSkipped++;
    }

    void printSummary() const {
        std::time_t endTime = std::time(nullptr);
        double durationSec = std::difftime(endTime, startTime);

        Logger::info("╔══════════════════════════════════════════╗");
        Logger::info("║         CRAWL SESSION SUMMARY            ║");
        Logger::info("╠══════════════════════════════════════════╣");
        Logger::info("║  Pages Downloaded:    " + padLeft(std::to_string(pagesDownloaded), 8) + "          ║");
        Logger::info("║  Failed Downloads:    " + padLeft(std::to_string(failedDownloads), 8) + "          ║");
        Logger::info("║  Robots.txt Blocked:  " + padLeft(std::to_string(robotsBlocked), 8) + "          ║");
        Logger::info("║  Duplicates Skipped:  " + padLeft(std::to_string(duplicatesSkipped), 8) + "          ║");
        Logger::info("║  Links Extracted:     " + padLeft(std::to_string(linksExtracted), 8) + "          ║");
        Logger::info("║  Total Bytes:         " + padLeft(formatBytes(totalBytesDownloaded), 8) + "          ║");
        Logger::info("║  Avg Page Size:       " + padLeft(avgPageSize(), 8) + "          ║");
        Logger::info("║  Duration:            " + padLeft(formatDuration(durationSec), 8) + "          ║");
        Logger::info("║  Pages/Second:        " + padLeft(pagesPerSec(durationSec), 8) + "          ║");
        Logger::info("╚══════════════════════════════════════════╝");
    }

private:
    // Right-align a string to a fixed width
    static std::string padLeft(const std::string& str, int width) {
        if ((int)str.size() >= width) return str;
        return std::string(width - str.size(), ' ') + str;
    }

    // Format bytes into a human-readable string
    static std::string formatBytes(long long bytes) {
        if (bytes < 1024) return std::to_string(bytes) + " B";
        if (bytes < 1024 * 1024) return std::to_string(bytes / 1024) + " KB";
        return std::to_string(bytes / (1024 * 1024)) + " MB";
    }

    // Format seconds into a human-readable duration
    static std::string formatDuration(double seconds) {
        if (seconds < 60) return std::to_string((int)seconds) + "s";
        int mins = (int)seconds / 60;
        int secs = (int)seconds % 60;
        return std::to_string(mins) + "m " + std::to_string(secs) + "s";
    }

    std::string avgPageSize() const {
        if (pagesDownloaded == 0) return "N/A";
        return formatBytes(totalBytesDownloaded / pagesDownloaded);
    }

    std::string pagesPerSec(double durationSec) const {
        if (durationSec <= 0) return "N/A";
        double pps = pagesDownloaded / durationSec;
        // Format to 1 decimal place
        int whole = (int)pps;
        int frac = (int)((pps - whole) * 10);
        return std::to_string(whole) + "." + std::to_string(frac);
    }
};

#endif // CRAWL_STATS_H
