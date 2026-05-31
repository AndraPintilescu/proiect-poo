#pragma once
#include <string>

// ── IObserver.h ───────────────────────────────────────────────────────────────
// Interfata Observer (Design Pattern Observer).
//
// Orice clasa care doreste sa fie notificata de evenimentele de cautare
// trebuie sa mosteneasca aceasta interfata si sa implementeze onSearch().
//
// Utilizare:
//   class MyLogger : public IObserver { ... };
//   index.addObserver(&myLogger);
// ─────────────────────────────────────────────────────────────────────────────

class IObserver {
public:
    // Destructor virtual – esential pentru stergerea corecta prin pointer la baza
    virtual ~IObserver() = default;

    // Apelat de Subject (Index) dupa fiecare cautare efectuata.
    // @param query   – sirul de cautare introdus de utilizator
    // @param results – numarul de documente gasite
    virtual void onSearch(const std::string& query, int results) = 0;
};
