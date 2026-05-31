// ── Logger.cpp ────────────────────────────────────────────────────────────────
// Implementarea clasei Logger (Observer concret).
// ─────────────────────────────────────────────────────────────────────────────

#include "Logger.h"

// ── Constructor ───────────────────────────────────────────────────────────────
// Deschide fisierul de log in modul append (ios::app) pentru a nu suprascrie
// intrarile precedente la fiecare rulare a programului.
Logger::Logger(const std::string& logFile)
    : logStream_(logFile, std::ios::app) {
    if (!logStream_.is_open())
        std::cerr << "[Logger] Atentie: nu s-a putut deschide fisierul '"
                  << logFile << "'.\n";
}

// ── Destructor ────────────────────────────────────────────────────────────────
// Inchide explicit stream-ul. Desi std::ofstream se inchide si in propriul
// destructor, o facem explicit pentru claritate.
Logger::~Logger() {
    if (logStream_.is_open())
        logStream_.close();
}

// ── onSearch ──────────────────────────────────────────────────────────────────
// Metoda apelata automat de Subject (Document) la fiecare cautare.
// Construieste un mesaj cu timestamp si il scrie atat la consola cat si
// in fisierul de log.
void Logger::onSearch(const std::string& query, int results) {
    std::string entry = "[" + timestamp() + "] CAUTARE: \"" + query +
                        "\" => " + std::to_string(results) + " rezultat(e)\n";
    std::cout << entry;
    if (logStream_.is_open())
        logStream_ << entry;
}

// ── timestamp ─────────────────────────────────────────────────────────────────
// Metoda statica privata: returneaza data/ora curenta ca string.
std::string Logger::timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return buf;
}
