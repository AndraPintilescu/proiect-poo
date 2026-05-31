#pragma once
#include "IObserver.h"
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>

// ── Logger.h ──────────────────────────────────────────────────────────────────
// Observer concret: inregistreaza fiecare cautare (cu timestamp) atat
// in fisierul de log, cat si la consola (stdout).
//
// Rolul in Design Pattern-ul Observer:
//   Logger este OBSERVER-ul concret care implementeaza IObserver.
//   Este notificat de Document (Subject) prin metoda onSearch().
//
// Utilizare:
//   Logger logger("search_log.txt");
//   index.addObserver(&logger);
//   // De acum, orice cautare va fi logata automat.
// ─────────────────────────────────────────────────────────────────────────────

class Logger : public IObserver {
public:
    // Deschide (sau creeaza) fisierul 'logFile' in modul append.
    // Daca fisierul nu poate fi deschis, afiseaza un avertisment la stderr.
    explicit Logger(const std::string& logFile = "search_log.txt");

    // Destructorul inchide fisierul de log daca este deschis.
    ~Logger() override;

    // Implementarea metodei din IObserver.
    // Scrie in fisier si la consola o linie de forma:
    //   [YYYY-MM-DD HH:MM:SS] SEARCH: "query" => N result(s)
    void onSearch(const std::string& query, int results) override;

private:
    std::ofstream logStream_;   // Stream catre fisierul de log

    // Returneaza data si ora curenta ca string formatat "YYYY-MM-DD HH:MM:SS".
    static std::string timestamp();
};
