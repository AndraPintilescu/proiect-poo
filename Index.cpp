// ── Index.cpp ─────────────────────────────────────────────────────────────────
// Implementarea clasei Index: incarcare documente, construirea indexului inversat,
// cautare simpla si avansata (AND/OR), eliminare stop-words, Observer pattern.
// ─────────────────────────────────────────────────────────────────────────────

#include "Index.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>   // C++17 – necesita compilare cu -std=c++17

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
// ── Stop-words ────────────────────────────────────────────────────────────────
// Lista de cuvinte frecvente in romana si engleza care NU se indexeaza,
// deoarece nu adauga valoare semantica unei cautari.
// ─────────────────────────────────────────────────────────────────────────────
const std::set<std::string> Index::STOP_WORDS = {
    // ── Romana ──
    "si", "in", "la", "de", "pe", "cu", "sa", "se", "ca", "nu",
    "un", "o",  "al", "ai", "ale", "lui", "ei", "lor", "sau",
    "dar", "ci", "daca", "mai", "tot", "cel", "cea", "cei", "cele",
    "este", "sunt", "era", "fost", "fi",
    "eu", "tu", "el", "ea", "noi", "voi",
    "ma", "te", "il", "le", "ne", "va",
    "din", "prin", "pentru", "spre", "despre", "intre",
    "astfel", "deci", "insa", "totusi", "chiar", "doar", "inca",
    "care", "cum", "cand", "unde", "cat",
    // ── Engleza ──
    "the", "a",  "an", "and", "or", "but", "in", "on", "at",
    "to",  "of", "for", "with", "by", "from", "up", "is", "are",
    "was", "were", "be", "been", "has", "have", "had", "do", "does",
    "did", "not", "that", "this", "it", "its", "as", "if",
    "he",  "she", "we",  "they", "you", "i",
    "can", "will", "just", "about", "which", "so", "also"
};

// ─────────────────────────────────────────────────────────────────────────────
// ── Destructor ────────────────────────────────────────────────────────────────
// Elibereaza memoria tuturor obiectelor Document alocate cu new.
// Observer-ii NU sunt stersi (Index nu le detine proprietatea).
// ─────────────────────────────────────────────────────────────────────────────
Index::~Index() {
    for (Document* doc : documents_)
        delete doc;
    documents_.clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Gestionarea Observer-ilor ─────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────

// Inregistreaza un observer.  Il adaugam si pe documentele deja incarcate,
// pentru ca apelul poate veni si dupa loadFromDirectory().
void Index::addObserver(IObserver* obs) {
    observers_.push_back(obs);
    for (Document* doc : documents_)
        doc->addObserver(obs);
}

// Elimina un observer din lista globala si din fiecare Document.
void Index::removeObserver(IObserver* obs) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), obs),
        observers_.end());
    for (Document* doc : documents_)
        doc->removeObserver(obs);
}

// Notifica observer-ii dupa o cautare:
//   - Daca exista documente, folosim primul Document ca "purtator" al notificarii
//     (el are deja lista de observer-i inregistrati).
//   - Altfel, apelam direct onSearch() pe fiecare observer din lista globala.
void Index::notifyAll(const std::string& query, int results) const {
    if (!documents_.empty()) {
        documents_.front()->notifyObservers(query, results);
    } else {
        for (IObserver* obs : observers_)
            obs->onSearch(query, results);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Metode auxiliare private ──────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────

// Converteste fiecare caracter din sirul 's' la litera mica.
// Folosim cast la unsigned char pentru a evita comportament nedefinit
// cu caracterele non-ASCII pe platforme unde char este signed.
std::string Index::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

// Imparte textul in tokeni:
//   - Parcurge caracter cu caracter.
//   - Acumuleaza secvente de litere (isalpha), convertite imediat la lowercase,
//     permitand si caractere speciale specifice limbajelor de programare (+ si #).
//   - Orice alt caracter non-litera functioneaza ca separator.
//   - Nu include in index tokeni formati exclusiv din simboluri (ex: "+", "++", "#").
//   - Rezultatul este un vector de cuvinte in lowercase.
std::vector<std::string> Index::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string currentToken;

    auto addTokenIfValid = [&](const std::string& token) {
        if (token.empty()) return;
        // Verificam daca tokenul contine cel putin o litera
        bool hasAlpha = false;
        for (char c : token) {
            if (std::isalpha(static_cast<unsigned char>(c)) != 0) {
                hasAlpha = true;
                break;
            }
        }
        if (hasAlpha) {
            tokens.push_back(token);
        }
    };

    for (unsigned char ch : text) {
        if (std::isalpha(ch) != 0 || ch == '+' || ch == '#') {
            // Caracter alfabetic sau simbol special acceptat
            currentToken += static_cast<char>(std::tolower(ch));
        } else {
            // Caracter separator: salvam tokenul acumulat (daca e valid)
            addTokenIfValid(currentToken);
            currentToken.clear();
        }
    }
    // Nu uitam ultimul token (daca textul nu se termina cu separator)
    addTokenIfValid(currentToken);

    return tokens;
}

// Returneaza true daca 'word' se afla in multimea STOP_WORDS.
// std::set::count este O(log n).
bool Index::isStopWord(const std::string& word) {
    return STOP_WORDS.count(word) > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// ── loadFromDirectory ─────────────────────────────────────────────────────────
// Citeste toate fisierele .txt din directorul specificat.
// Fiecare fisier este citit integral si incapsulat intr-un obiect Document.
// Observer-ii inregistrati anterior sunt atasati si documentelor noi.
// ─────────────────────────────────────────────────────────────────────────────
void Index::loadFromDirectory(const std::string& dirPath) {
    // Verificam ca directorul exista
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "[Index] Eroare: '" << dirPath
                  << "' nu este un director valid.\n";
        return;
    }

    int loaded = 0;
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        // Procesam doar fisierele regulate cu extensia .txt
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".txt") continue;

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            std::cerr << "[Index] Atentie: nu s-a putut deschide "
                      << entry.path() << "\n";
            continue;
        }

        // Citim intregul continut al fisierului intr-un string
        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Cream documentul si il inregistram ca proprietate a Index-ului
        Document* doc = new Document(entry.path().string(), content);

        // Atasam observer-ii deja inregistrati si acestui document nou
        for (IObserver* obs : observers_)
            doc->addObserver(obs);

        documents_.push_back(doc);
        ++loaded;
    }

    std::cout << "[Index] Incarcate " << loaded
              << " fisier(e) din '" << dirPath << "'.\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// ── buildIndex ────────────────────────────────────────────────────────────────
// Construieste indexul inversat:
//   1. Tokenizeaza continutul fiecarui Document.
//   2. Elimina stop-words.
//   3. Pentru fiecare cuvant ramas, adauga Document* in lista asociata,
//      evitand duplicatele (un document apare o singura data per cuvant).
// ─────────────────────────────────────────────────────────────────────────────
void Index::buildIndex() {
    index_.clear();  // curatam indexul anterior (daca exista)

    int stopWordsRemoved = 0;
    int totalTokens      = 0;

    for (Document* doc : documents_) {
        std::vector<std::string> tokens = tokenize(doc->content);
        totalTokens += static_cast<int>(tokens.size());

        for (const std::string& word : tokens) {
            // ── Eliminare stop-words ──────────────────────────────────
            if (isStopWord(word)) {
                ++stopWordsRemoved;
                continue;
            }

            // Obtinem (sau cream) lista de documente pentru acest cuvant
            std::vector<Document*>& docList = index_[word];

            // Evitam adaugarea aceluiasi document de mai multe ori
            bool alreadyPresent = false;
            for (Document* existing : docList) {
                if (existing == doc) {
                    alreadyPresent = true;
                    break;
                }
            }
            if (!alreadyPresent)
                docList.push_back(doc);
        }
    }

    std::cout << "[Index] Index construit. Cuvinte unice indexate: "
              << index_.size()
              << ". Tokeni eliminati (stop-words): "
              << stopWordsRemoved << " din " << totalTokens << " total.\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// ── search – un singur cuvant ─────────────────────────────────────────────────
// Cauta un cuvant in index (case-insensitive).
// Notifica observer-ii cu rezultatul cautarii.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<Document*> Index::search(const std::string& word) const {
    std::string normalised = toLower(word);
    std::vector<Document*> results;

    auto it = index_.find(normalised);
    if (it != index_.end())
        results = it->second;

    // Notificare Observer – Logger va inregistra aceasta cautare
    notifyAll(word, static_cast<int>(results.size()));
    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// ── search – cautare avansata AND / OR ───────────────────────────────────────
// Tokenizeaza sirul 'query' si aplica logica AND sau OR.
//
// AND (intersectie):
//   Incepe cu lista documentelor care contin primul cuvant, apoi
//   filtreaza pastrand doar documentele prezente in TOATE listele.
//
// OR (reuniune):
//   Colecteaza toate documentele care contin CEL PUTIN UN cuvant,
//   folosind un std::set<Document*> pentru deduplicare.
// ─────────────────────────────────────────────────────────────────────────────
std::vector<Document*> Index::search(const std::string& query, SearchMode mode) const {
    std::vector<std::string> words = tokenize(query);

    if (words.empty()) {
        notifyAll(query, 0);
        return {};
    }

    std::vector<Document*> results;

    if (mode == SearchMode::OR) {
        // ── Reuniune ──────────────────────────────────────────────────────
        std::set<Document*> seen;  // set pentru deduplicare eficienta
        for (const std::string& w : words) {
            auto it = index_.find(w);
            if (it == index_.end()) continue;
            for (Document* doc : it->second) {
                // insert returneaza {iterator, bool}; bool=true => element nou
                if (seen.insert(doc).second)
                    results.push_back(doc);
            }
        }
    } else {
        // ── Intersectie (AND implicit) ────────────────────────────────────
        // Setul candidat initial = documentele care contin primul cuvant
        auto it = index_.find(words[0]);
        if (it == index_.end()) {
            notifyAll(query, 0);
            return {};
        }
        results = it->second;

        // Filtram succesiv pentru fiecare cuvant urmator
        for (std::size_t i = 1; i < words.size() && !results.empty(); ++i) {
            auto it2 = index_.find(words[i]);
            if (it2 == index_.end()) {
                // Niciun document nu contine cuvantul curent => intersectie vida
                results.clear();
                break;
            }
            const std::vector<Document*>& nextList = it2->second;

            // Pastram doar documentele prezente in ambele liste
            std::vector<Document*> intersection;
            intersection.reserve(results.size());
            for (Document* doc : results) {
                bool found = false;
                for (Document* d2 : nextList)
                    if (d2 == doc) { found = true; break; }
                if (found)
                    intersection.push_back(doc);
            }
            results = intersection;
        }
    }

    // Notificare Observer dupa finalizarea cautarii
    notifyAll(query, static_cast<int>(results.size()));
    return results;
}

// ─────────────────────────────────────────────────────────────────────────────
// ── printIndex ────────────────────────────────────────────────────────────────
// Afiseaza toate cuvintele din index si numarul de documente asociate.
// Util pentru debugging si demonstratii.
// ─────────────────────────────────────────────────────────────────────────────
void Index::printIndex() const {
    std::cout << "\n=== Index Inversat (" << index_.size() << " cuvinte) ===\n";
    for (const auto& [word, docs] : index_)
        std::cout << "  " << word << " (" << docs.size() << " doc(uri))\n";
    std::cout << "==========================================\n\n";
}
