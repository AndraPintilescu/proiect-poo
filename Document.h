#pragma once
#include <string>
#include <vector>
#include "IObserver.h"

// ── Document.h ────────────────────────────────────────────────────────────────
// Reprezinta un document text incarcat de pe disc.
//
// Rolul in Design Pattern-ul Observer:
//   Document actioneaza ca SUBJECT (Observabil).
//   Mentine o lista de pointeri la IObserver si ii notifica
//   prin apelul notifyObservers() atunci cand se efectueaza o cautare.
//
//   In aceasta implementare, Index apeleaza doc->notifyObservers() dupa
//   fiecare cautare, delegand responsabilitatea notificarii catre Document.
// ─────────────────────────────────────────────────────────────────────────────

class Document {
public:
    // ── Atribute publice ──────────────────────────────────────────────────
    std::string filePath;   // Calea completa catre fisierul .txt
    std::string content;    // Continutul brut al fisierului

    // Constructor
    Document(const std::string& path, const std::string& text);

    // ── Gestionarea Observer-ilor ─────────────────────────────────────────
    // Adauga un observer in lista (nu preia proprietatea pointerului)
    void addObserver(IObserver* obs);

    // Elimina un observer din lista (nu sterge obiectul)
    void removeObserver(IObserver* obs);

    // Notifica toti observer-ii ca s-a efectuat o cautare
    // @param query   – cuvantul/interogarea cautata
    // @param results – numarul de rezultate gasite
    void notifyObservers(const std::string& query, int results) const;

    // Afiseaza un rezumat scurt al documentului (cale + previzualizare contextuala)
    void print(const std::string& query = "") const;

private:
    // Lista de observer-i inregistrati (nu detine proprietatea pointerilor)
    std::vector<IObserver*> observers_;
};
