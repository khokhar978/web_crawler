#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

// Log severity levels
enum class LogLevel {
    INFO,
    WARN,
    ERROR,
    DEBUG
};

// A simple static Logger that outputs timestamped, leveled messages
// to both the console and a persistent log file simultaneously.
class Logger {
private:
    static std::ofstream logFile;
    static bool fileOpen;

    // Returns a formatted timestamp string like "2025-07-17 14:30:05"
    static std::string getTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
        return std::string(buffer);
    }

    // Converts enum to a padded string tag
    static std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARN:  return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::DEBUG: return "DEBUG";
            default:              return "?????";
        }
    }

public:
    // Initialize the log file. Call once at startup.
    static void init(const std::string& filename) {
        logFile.open(filename, std::ios::app);
        if (logFile.is_open()) {
            fileOpen = true;
            log(LogLevel::INFO, "=== Logger initialized. Session started. ===");
        }
    }

    // Core logging function: formats and outputs to console + file
    static void log(LogLevel level, const std::string& message) {
        std::string formatted = "[" + getTimestamp() + "] [" + levelToString(level) + "] " + message;
        
        // Always print to console
        std::cout << formatted << "\n";
        
        // Also write to the log file if it's open
        if (fileOpen) {
            logFile << formatted << "\n";
            logFile.flush();
        }
    }

    // Convenience wrappers
    static void info(const std::string& message)  { log(LogLevel::INFO, message);  }
    static void warn(const std::string& message)   { log(LogLevel::WARN, message);  }
    static void error(const std::string& message) { log(LogLevel::ERROR, message); }
    static void debug(const std::string& message) { log(LogLevel::DEBUG, message); }

    // Clean shutdown
    static void shutdown() {
        if (fileOpen) {
            log(LogLevel::INFO, "=== Logger shutting down. Session ended. ===");
            logFile.close();
            fileOpen = false;
        }
    }
};

#endif // LOGGER_H
