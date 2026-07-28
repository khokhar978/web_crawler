#ifndef REDISCLI_H
#define REDISCLI_H

#include <string>
#include "HashMap.h"

class RedisCLI {
private:
    HashMap<std::string, std::string> db;

public:
    RedisCLI() = default;

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key) const;
    int count() const;
    void clear();
    void run();
};

#endif // REDISCLI_H
