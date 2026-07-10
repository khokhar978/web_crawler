#include <iostream>
#include "sqlite3.h"

int main() {
    std::cout << "Crawler started." << std::endl;
    std::cout << "SQLite version successfully linked: " << sqlite3_libversion() << std::endl;
    return 0;
}
