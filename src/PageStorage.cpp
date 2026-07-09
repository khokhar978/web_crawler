#include "PageStorage.h"
#include <iostream>

PageStorage::PageStorage(const std::string& archivePath, const std::string& dbPath) 
    : archiveFilePath(archivePath), dbFilePath(dbPath) {
    
    // 1. Open the archive file in append mode. 
    // We use binary mode to ensure raw HTML bytes aren't modified by OS newline conversions.
    archiveFile.open(archiveFilePath, std::ios::app | std::ios::binary);
    
    if (!archiveFile.is_open()) {
        std::cerr << "Failed to open archive file: " << archiveFilePath << std::endl;
    }

    // TODO: Connect to SQLite database
    // TODO: Call initDatabase() to ensure the crawler_metadata table exists
}

PageStorage::~PageStorage() {
    // 1. Safely close the archive file when the crawler shuts down
    if (archiveFile.is_open()) {
        archiveFile.close();
    }
    
    // TODO: Close the SQLite database connection
}

void PageStorage::initDatabase() {
    // TODO: Execute CREATE TABLE IF NOT EXISTS crawler_metadata ...
}

void PageStorage::storePage(const std::string& url, const std::string& html, int depth) {
    if (!archiveFile.is_open()) {
        std::cerr << "Error: Archive file is not open!" << std::endl;
        return;
    }

    // 1. Get the current write position (offset) of the archive file before we write anything
    // We cast to long to ensure we have enough capacity for massive files
    long offset = static_cast<long>(archiveFile.tellp());
    
    // 2. Write the raw HTML directly to the end of the archive file
    archiveFile.write(html.c_str(), html.length());
    
    // Force a flush so the data hits the hard drive immediately (useful if the crawler crashes)
    archiveFile.flush();
    
    // 3. Insert the url, depth, offset, and html.length() into the SQLite database
    // TODO: We will implement the SQLite INSERT query here next!
}

std::string PageStorage::getPage(const std::string& url) {
    // TODO: 1. Query SQLite for the offset and length using the URL
    // TODO: 2. Seek to that offset in the archive file
    // TODO: 3. Read 'length' bytes and return the string
    return ""; // Placeholder
}

bool PageStorage::hasPage(const std::string& url) {
    // TODO: Query SQLite to check if a row with this URL exists
    return false; // Placeholder
}

std::string PageStorage::getURLByID(int id) {
    // TODO: Query SQLite for the URL where the Primary Key == id
    return ""; // Placeholder
}

int PageStorage::pageCount() {
    // TODO: Execute SELECT COUNT(*) FROM crawler_metadata
    return 0; // Placeholder
}
