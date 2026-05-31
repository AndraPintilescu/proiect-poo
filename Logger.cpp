#include "Logger.h"

Logger::Logger(const std::string& logFile)
    : logStream_(logFile, std::ios::app) {
    if (!logStream_.is_open())
        std::cerr << "[Logger] Atentie: nu s-a putut deschide fisierul '"
                  << logFile << "'.\n";
}

Logger::~Logger() {
    if (logStream_.is_open())
        logStream_.close();
}

void Logger::onSearch(const std::string& query, int results) {
    std::string entry = "[" + timestamp() + "] CAUTARE: \"" + query +
                        "\" => " + std::to_string(results) + " rezultat(e)\n";
    std::cout << entry;
    if (logStream_.is_open())
        logStream_ << entry;
}

std::string Logger::timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}
