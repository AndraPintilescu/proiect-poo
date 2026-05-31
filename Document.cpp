// ── Document.cpp ──────────────────────────────────────────────────────────────
// Implementarea clasei Document.
// ─────────────────────────────────────────────────────────────────────────────

#include "Document.h"
#include <iostream>
#include <algorithm>  // std::remove
#include <cctype>     // std::tolower

// ── Constructor ───────────────────────────────────────────────────────────────
// Initializeaza calea si continutul documentului.
Document::Document(const std::string& path, const std::string& text)
    : filePath(path), content(text) {}

// ── addObserver ───────────────────────────────────────────────────────────────
// Inregistreaza un observer in lista interna.
// Nu preia proprietatea pointerului – nu apeleaza delete.
void Document::addObserver(IObserver* obs) {
    observers_.push_back(obs);
}

// ── removeObserver ────────────────────────────────────────────────────────────
// Elimina un observer din lista (prin comparatie de pointer).
// Foloseste idiomul erase-remove pentru stergere eficienta.
void Document::removeObserver(IObserver* obs) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), obs),
        observers_.end());
}

// ── notifyObservers ───────────────────────────────────────────────────────────
// Parcurge lista de observer-i si apeleaza onSearch() pe fiecare.
// Aceasta este metoda prin care Subject-ul (Document) notifica observer-ii.
void Document::notifyObservers(const std::string& query, int results) const {
    for (IObserver* obs : observers_)
        obs->onSearch(query, results);
}

#include <sstream>
#include <vector>

// Helper pentru a converti un string la litere mici
static std::string toLowerStr(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return res;
}

// Helper pentru verificarea case-insensitive a unui cuvant intr-o linie
static bool containsWordIgnoreCase(const std::string& line, const std::string& word) {
    if (word.empty()) return false;
    std::string lowerLine = toLowerStr(line);
    std::string lowerWord = toLowerStr(word);
    return lowerLine.find(lowerWord) != std::string::npos;
}

// Helper pentru a extrage cuvintele cheie din interogare (excluzand AND/OR)
static std::vector<std::string> getQueryWords(const std::string& query) {
    std::vector<std::string> words;
    std::stringstream ss(query);
    std::string word;
    while (ss >> word) {
        std::string lower = toLowerStr(word);
        if (lower != "and" && lower != "or") {
            words.push_back(lower);
        }
    }
    return words;
}

// ── print ─────────────────────────────────────────────────────────────────────
// Afiseaza calea documentului si o previzualizare contextuala (linia care contine
// cuvintele cautate, sau primele 120 de caractere daca nu se cauta nimic specific).
void Document::print(const std::string& query) const {
    std::cout << "  [Document] " << filePath << "\n";
    
    std::string preview;
    bool foundPreview = false;

    if (!query.empty()) {
        std::vector<std::string> searchWords = getQueryWords(query);
        if (!searchWords.empty()) {
            std::stringstream docStream(content);
            std::string line;
            while (std::getline(docStream, line)) {
                // Verificam daca linia contine cel putin unul dintre cuvintele cautate
                for (const std::string& w : searchWords) {
                    if (containsWordIgnoreCase(line, w)) {
                        preview = line;
                        foundPreview = true;
                        break;
                    }
                }
                if (foundPreview) break;
            }
        }
    }

    if (!foundPreview) {
        preview = content.substr(0, 120);
        if (content.size() > 120)
            preview += "...";
    } else {
        // Curatam eventualele caractere de retur la rand (\r) de la sfarsitul liniei (specifice Windows)
        if (!preview.empty() && preview.back() == '\r') {
            preview.pop_back();
        }
        // Daca linia gasita este foarte lunga, o trunchiem pentru un preview frumos
        if (preview.size() > 120) {
            preview = preview.substr(0, 120) + "...";
        }
    }

    std::cout << "  Preview  : " << preview << "\n";
}
