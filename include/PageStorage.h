#ifndef PAGE_STORAGE_H
#define PAGE_STORAGE_H

#include <string>
#include <fstream>
#include "sqlite3.h"

// Hybrid Storage: SQL Database (crawler_metadata) + Append-Only File (crawler_archive.dat)
class PageStorage {
private:
    std::string archiveFilePath;
    std::string dbFilePath;
    std::ofstream archiveFile;
    sqlite3* db; // Pointer to our SQLite connection

    // Helper method to setup the SQLite database table
    void initDatabase();

public:
    PageStorage(const std::string& archivePath = "crawler_archive.dat", 
                const std::string& dbPath = "crawler.db");
    ~PageStorage();
    
    // Project 03 required API
    void storePage(const std::string& url, const std::string& html, int depth);
    std::string getPage(const std::string& url);
    bool hasPage(const std::string& url);
    std::string getURLByID(int id);
    int pageCount();
};

#endif // PAGE_STORAGE_H
