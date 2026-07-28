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

    // 2. Connect to SQLite database
    int rc = sqlite3_open(dbFilePath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to open SQLite database: " << sqlite3_errmsg(db) << std::endl;
    } else {
        // 3. Ensure the metadata table exists
        initDatabase();
    }
}

PageStorage::~PageStorage() {
    // 1. Safely close the archive file when the crawler shuts down
    if (archiveFile.is_open()) {
        archiveFile.close();
    }
    
    // 2. Close the SQLite database connection
    if (db != nullptr) {
        sqlite3_close(db);
    }
}

void PageStorage::initDatabase() {
    // Execute CREATE TABLE IF NOT EXISTS crawler_metadata
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS crawler_metadata ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "url TEXT UNIQUE, "
        "depth INTEGER, "
        "offset INTEGER, "
        "length INTEGER);";
        
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error during initDatabase: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

void PageStorage::storePage(const std::string& url, const std::string& html, int depth) {
    if (!archiveFile.is_open()) {
        std::cerr << "Error: Archive file is not open!" << std::endl;
        return;
    }

    // 1. Get the current write position (offset) of the archive file before we write anything
    // We cast to long to ensure we have enough capacity for massive files
    long long offset = static_cast<long>(archiveFile.tellp());
    
    // 2. Write the raw HTML directly to the end of the archive file
    archiveFile.write(html.c_str(), html.length());
    
    // Force a flush so the data hits the hard drive immediately (useful if the crawler crashes)
    archiveFile.flush();
    
    // 3. Insert the url, depth, offset, and html.length() into the SQLite database. 
    // We use IGNORE so that if we re-crawl a URL, we don't delete the old row (which breaks the sequential IDs for Project 3).
    const char* sql = "INSERT OR IGNORE INTO crawler_metadata (url, depth, offset, length) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    // Bind the parameters safely to prevent SQL injection
    sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, depth);
    sqlite3_bind_int64(stmt, 3, offset);
    sqlite3_bind_int64(stmt, 4, html.length());
    
    // Execute the query
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to insert page metadata: " << sqlite3_errmsg(db) << std::endl;
    }
    
    // Clean up the prepared statement
    sqlite3_finalize(stmt);
}

std::string PageStorage::getPage(const std::string& url) {
    // 1. Query SQLite for the offset and length using the URL
    const char* sql = "SELECT offset, length FROM crawler_metadata WHERE url = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
        return "";
    }
    
    sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_TRANSIENT);
    
    std::string html = "";
    
    // If the query finds a matching row
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        long offset = sqlite3_column_int64(stmt, 0);
        long length = sqlite3_column_int64(stmt, 1);
        
        // 2. Open an input stream specifically for reading
        std::ifstream reader(archiveFilePath, std::ios::binary);
        if (reader.is_open()) {
            // 3. Seek to that exact byte offset in the massive file
            reader.seekg(offset, std::ios::beg);
            
            // 4. Read precisely 'length' bytes
            // We use a vector buffer to safely hold the bytes before turning it into a string
            std::string buffer;
            buffer.resize(length);
            reader.read(&buffer[0], length);
            
            html = buffer;
            reader.close();
        } else {
            std::cerr << "Failed to open archive file for reading: " << archiveFilePath << std::endl;
        }
    }
    
    sqlite3_finalize(stmt);
    return html;
}

bool PageStorage::hasPage(const std::string& url) {
    // Query SQLite to check if a row with this URL exists
    const char* sql = "SELECT 1 FROM crawler_metadata WHERE url = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_TRANSIENT);
    
    // If SQLITE_ROW is returned, it means at least one matching row was found!
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    
    sqlite3_finalize(stmt);
    return exists;
}

std::string PageStorage::getURLByID(int id) {
    const char* sql = "SELECT url FROM crawler_metadata WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return "";
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    std::string url = "";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // SQLite returns a const unsigned char*, so we cast it to const char* for std::string
        url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return url;
}

int PageStorage::pageCount() {
    const char* sql = "SELECT COUNT(*) FROM crawler_metadata;";
    sqlite3_stmt* stmt;
    
    int count = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return count;
}

DynamicArray<std::string> PageStorage::getAllSeenURLs() {
    DynamicArray<std::string> urls;
    const char* sql = "SELECT url FROM crawler_metadata;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare select all urls statement: " << sqlite3_errmsg(db) << std::endl;
        return urls;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* urlText = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (urlText) {
            urls.append(std::string(urlText));
        }
    }
    
    sqlite3_finalize(stmt);
    return urls;
}
