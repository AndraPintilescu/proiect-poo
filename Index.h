#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include "Document.h"
#include "IObserver.h"

// ── Index.h ───────────────────────────────────────────────────────────────────
// Clasa principala a motorului de cautare.
// Construieste si interogheaza un index inversat peste o colectie de documente.
//
// ── Structura interna ────────────────────────────────────────────────────────
//   std::map<std::string, std::vector<Document*>> index_
//   └── cuvant ──► lista de Document* care contin acel cuvant
//
// ── Relatia cu Design Pattern-ul Observer ────────────────────────────────────
//   Index este cel care coordoneaza cautarile.
//   Dupa fiecare cautare apeleaza notifyAll() care la randul sau apeleaza
//   notifyObservers() pe Document-ul responsabil (primul document),
//   propagand evenimentul catre toti observer-ii (ex: Logger).
//
//   Index gestioneaza si lista de observer-i globali, pe care ii distribuie
//   tuturor Document-elor incarcate, astfel incat notificarea sa fie corecta.
//
// ── Fluxul de utilizare ──────────────────────────────────────────────────────
//   Index idx;
//   Logger logger;
//   idx.addObserver(&logger);          // inregistreaza observer
//   idx.loadFromDirectory("docs/");    // incarca documente
//   idx.buildIndex();                  // construieste indexul
//   auto r = idx.search("pisica");     // cauta (logger e notificat automat)
// ─────────────────────────────────────────────────────────────────────────────

// ── Modul de cautare ──────────────────────────────────────────────────────────
enum class SearchMode {
    AND,    // toate cuvintele trebuie sa apara in document (intersectie)
    OR      // cel putin un cuvant trebuie sa apara (reuniune)
};

class Index {
public:
    Index() = default;
    ~Index();   // elibereaza toate obiectele Document alocate dinamic

    // ── Gestionarea Observer-ilor ─────────────────────────────────────────
    // Inregistreaza un observer la toate documentele existente si viitoare.
    void addObserver(IObserver* obs);

    // Elimina un observer din toate documentele.
    void removeObserver(IObserver* obs);

    // ── Incarcarea datelor ────────────────────────────────────────────────
    // Citeste toate fisierele .txt din directorul 'dirPath'.
    // Foloseste std::filesystem (C++17).
    void loadFromDirectory(const std::string& dirPath);

    // ── Construirea indexului ─────────────────────────────────────────────
    // Tokenizeaza continutul fiecarui Document, elimina stop-words si
    // populeaza indexul inversat.  Apelati dupa loadFromDirectory().
    void buildIndex();

    // ── Cautare (un singur cuvant, case-insensitive) ──────────────────────
    std::vector<Document*> search(const std::string& word) const;

    // ── Cautare avansata (AND / OR) ───────────────────────────────────────
    // 'query' poate contine mai multe cuvinte separate prin spatii.
    // Modul AND: intersectia listelor; OR: reuniunea listelor.
    std::vector<Document*> search(const std::string& query, SearchMode mode) const;

    // Afiseaza toate cuvintele din index (util la debugging).
    void printIndex() const;

private:
    // ── Date membre ───────────────────────────────────────────────────────
    std::vector<Document*>                      documents_;  // proprietar al Document*
    std::map<std::string, std::vector<Document*>> index_;    // indexul inversat
    std::vector<IObserver*>                     observers_;  // observer-i globali

    // ── Stop-words ────────────────────────────────────────────────────────
    // Cuvinte frecvente (romana + engleza) care NU se indexeaza.
    static const std::set<std::string> STOP_WORDS;

    // ── Metode auxiliare private ──────────────────────────────────────────
    // Converteste un string la litere mici (lowercase).
    static std::string toLower(const std::string& s);

    // Imparte textul in tokeni: secvente de litere, convertite la lowercase.
    // Semnele de punctuatie si spatiile sunt considerate delimitatori.
    static std::vector<std::string> tokenize(const std::string& text);

    // Verifica daca un cuvant se afla in lista de stop-words.
    static bool isStopWord(const std::string& word);

    // Notifica toti observer-ii dupa o cautare (prin Document-ul front sau direct).
    void notifyAll(const std::string& query, int results) const;
};
