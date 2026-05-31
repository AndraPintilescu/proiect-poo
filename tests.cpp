// ── tests.cpp ─────────────────────────────────────────────────────────────────
// Teste unitare scrise manual pentru validarea logicii de indexare si cautare.
//
// Framework de testare: minimal, implementat in acest fisier (fara dependente externe).
// Macro-urile ASSERT_TRUE si ASSERT_EQ afiseaza PASS/FAIL pentru fiecare verificare.
//
// Compilare separata (g++):
//   g++ -std=c++17 -o tests tests.cpp Document.cpp Index.cpp Logger.cpp
//
// Compilare separata (MSVC):
//   cl /std:c++17 /EHsc tests.cpp Document.cpp Index.cpp Logger.cpp /Fe:tests.exe
//
// IMPORTANT: tests.cpp are propriul main() si se compileaza SEPARAT de main.cpp.
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include "Document.h"
#include "Index.h"
#include "Logger.h"

namespace fs = std::filesystem;

// ─────────────────────────────────────────────────────────────────────────────
// ── Framework minimal de testare ─────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────

static int testsRun    = 0;
static int testsPassed = 0;
static int testsFailed = 0;

// Verifica ca expresia 'expr' este adevarata.
#define ASSERT_TRUE(expr)                                                         \
    do {                                                                          \
        ++testsRun;                                                                \
        if (!(expr)) {                                                             \
            std::cerr << "  [FAIL] " << #expr                                     \
                      << "  (" << __FILE__ << ":" << __LINE__ << ")\n";           \
            ++testsFailed;                                                         \
        } else {                                                                   \
            std::cout << "  [PASS] " << #expr << "\n";                            \
            ++testsPassed;                                                         \
        }                                                                          \
    } while (false)

// Verifica egalitatea dintre 'a' si 'b' si afiseaza valorile la FAIL.
#define ASSERT_EQ(a, b)                                                           \
    do {                                                                          \
        ++testsRun;                                                                \
        if ((a) != (b)) {                                                          \
            std::cerr << "  [FAIL] " << #a << " == " << #b                        \
                      << "  (obtinut: " << (a) << " vs " << (b) << ")"           \
                      << "  (" << __FILE__ << ":" << __LINE__ << ")\n";           \
            ++testsFailed;                                                         \
        } else {                                                                   \
            std::cout << "  [PASS] " << #a << " == " << #b << "\n";              \
            ++testsPassed;                                                         \
        }                                                                          \
    } while (false)

// ─────────────────────────────────────────────────────────────────────────────
// ── Utilitare pentru teste ────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────

// Creeaza un director temporar cu fisiere .txt pentru testare.
// Returneaza calea directorului creat.
static std::string createTempDocs() {
    std::string dir = "test_docs_tmp";
    fs::create_directories(dir);

    // Scriem fisiere cu continut predictibil pentru teste
    auto write = [&](const std::string& name, const std::string& content) {
        std::ofstream f(dir + "/" + name);
        f << content;
    };

    // alpha.txt: contine "fox", "quick", "brown", "dog", "lazy"
    write("alpha.txt", "the quick brown fox jumps over the lazy dog");

    // beta.txt: contine "fox", "forest", "rabbit"
    write("beta.txt",  "a fox in the forest hunts for rabbit");

    // gamma.txt: contine "programming", "cpp", "attention", "detail"
    write("gamma.txt", "programming in cpp requires attention to detail");

    return dir;
}

// Sterge directorul temporar dupa test.
static void removeTempDocs(const std::string& dir) {
    fs::remove_all(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 1: Document – atribute de baza ─────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void test_document_basic() {
    std::cout << "\n[Suite 1] Document – atribute de baza\n";

    Document doc("cale/fisier.txt", "Salut lume");

    // Verificam ca atributele sunt setate corect de constructor
    ASSERT_EQ(doc.filePath, std::string("cale/fisier.txt"));
    ASSERT_EQ(doc.content,  std::string("Salut lume"));
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 2: Document – Design Pattern Observer ───────────────────────────────
// Verifica ca addObserver / removeObserver / notifyObservers functioneaza corect.
// ─────────────────────────────────────────────────────────────────────────────
void test_document_observer() {
    std::cout << "\n[Suite 2] Document – Observer pattern\n";

    // Observer mock local pentru a captura apelurile onSearch()
    struct MockObserver : IObserver {
        std::string lastQuery;
        int         lastResults = -1;
        int         callCount   = 0;

        void onSearch(const std::string& q, int r) override {
            lastQuery   = q;
            lastResults = r;
            ++callCount;
        }
    };

    Document doc("test.txt", "continut");
    MockObserver obs;

    // Test: notificare dupa addObserver
    doc.addObserver(&obs);
    doc.notifyObservers("pisica", 3);

    ASSERT_EQ(obs.callCount,   1);
    ASSERT_EQ(obs.lastQuery,   std::string("pisica"));
    ASSERT_EQ(obs.lastResults, 3);

    // Test: fara notificare dupa removeObserver
    doc.removeObserver(&obs);
    doc.notifyObservers("caine", 1);
    ASSERT_EQ(obs.callCount, 1);  // contorul ramane la 1, nu a mai fost apelat

    // Test: mai multi observer-i
    MockObserver obs2;
    doc.addObserver(&obs);
    doc.addObserver(&obs2);
    doc.notifyObservers("test", 5);

    ASSERT_EQ(obs.callCount,  2);  // obs a mai primit o notificare
    ASSERT_EQ(obs2.callCount, 1);  // obs2 si-a primit prima notificare
    ASSERT_EQ(obs2.lastResults, 5);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 3: Index – incarcare si construire index ────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void test_index_load_and_build() {
    std::cout << "\n[Suite 3] Index – incarcare si construire index\n";

    std::string dir = createTempDocs();

    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    // "fox" apare in alpha.txt si beta.txt => 2 documente
    auto res = index.search("fox");
    ASSERT_EQ(res.size(), std::size_t(2));

    // "programming" apare doar in gamma.txt => 1 document
    auto res2 = index.search("programming");
    ASSERT_EQ(res2.size(), std::size_t(1));

    // Cuvant inexistent => 0 documente
    auto res3 = index.search("unicorn");
    ASSERT_TRUE(res3.empty());

    removeTempDocs(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 4: Index – eliminare stop-words ────────────────────────────────────
// Verifica ca stop-words-urile NU sunt indexate.
// ─────────────────────────────────────────────────────────────────────────────
void test_index_stop_words_removed() {
    std::cout << "\n[Suite 4] Index – eliminare stop-words\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    // Cuvintele din stop-list NU trebuie sa apara in index
    ASSERT_TRUE(index.search("the").empty());   // englez
    ASSERT_TRUE(index.search("a").empty());     // englez
    ASSERT_TRUE(index.search("in").empty());    // englez + roman
    ASSERT_TRUE(index.search("for").empty());   // englez
    ASSERT_TRUE(index.search("to").empty());    // englez

    removeTempDocs(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 5: Index – cautare AND (intersectie) ───────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void test_index_search_and() {
    std::cout << "\n[Suite 5] Index – cautare AND\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    // "fox" AND "forest" => doar beta.txt contine ambele
    auto res = index.search("fox forest", SearchMode::AND);
    ASSERT_EQ(res.size(), std::size_t(1));

    // "fox" AND "programming" => niciun document nu le contine pe ambele
    auto res2 = index.search("fox programming", SearchMode::AND);
    ASSERT_TRUE(res2.empty());

    // "fox" AND "quick" => doar alpha.txt le contine pe ambele
    auto res3 = index.search("fox quick", SearchMode::AND);
    ASSERT_EQ(res3.size(), std::size_t(1));

    // Cuvant inexistent in AND => rezultat vid
    auto res4 = index.search("fox dragon", SearchMode::AND);
    ASSERT_TRUE(res4.empty());

    removeTempDocs(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 6: Index – cautare OR (reuniune) ───────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void test_index_search_or() {
    std::cout << "\n[Suite 6] Index – cautare OR\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    // "fox" apare in alpha + beta (2 doc), "programming" in gamma (1 doc)
    // OR => reuniune = 3 documente distincte
    auto res = index.search("fox programming", SearchMode::OR);
    ASSERT_EQ(res.size(), std::size_t(3));

    // Ambele cuvinte inexistente => 0 documente
    auto res2 = index.search("unicorn dragon", SearchMode::OR);
    ASSERT_TRUE(res2.empty());

    // Un cuvant valid, unul inexistent => doar documentele cuvantului valid
    auto res3 = index.search("rabbit unicorn", SearchMode::OR);
    ASSERT_EQ(res3.size(), std::size_t(1));  // doar beta.txt (rabbit)

    removeTempDocs(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 7: Index – cautare case-insensitive ─────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
void test_index_case_insensitive() {
    std::cout << "\n[Suite 7] Index – cautare case-insensitive\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    // "fox", "FOX", "FoX" trebuie sa returneze acelasi numar de documente
    auto lower = index.search("fox");
    auto upper = index.search("FOX");
    auto mixed = index.search("FoX");

    ASSERT_EQ(lower.size(), upper.size());
    ASSERT_EQ(lower.size(), mixed.size());

    // Verificam ca gasim corect 2 documente indiferent de majuscule
    ASSERT_EQ(lower.size(), std::size_t(2));

    removeTempDocs(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 8: Logger – integrare cu Observer ───────────────────────────────────
// Verifica ca Logger-ul scrie corect in fisier la fiecare cautare.
// ─────────────────────────────────────────────────────────────────────────────
void test_logger_observer_integration() {
    std::cout << "\n[Suite 8] Logger – integrare Observer\n";

    std::string dir     = createTempDocs();
    std::string logFile = "test_search_log_tmp.txt";

    // Stergem eventualul fisier vechi de la o rulare anterioara
    fs::remove(logFile);

    {
        Index  index;
        Logger logger(logFile);
        index.addObserver(&logger);
        index.loadFromDirectory(dir);
        index.buildIndex();

        index.search("fox");
        index.search("programming");
        index.search("unicorn");     // cautare fara rezultate
    }  // Logger se distruge aici si inchide fisierul

    // Fisierul de log trebuie sa existe
    ASSERT_TRUE(fs::exists(logFile));

    // Citim continutul log-ului
    std::ifstream lf(logFile);
    std::string content((std::istreambuf_iterator<char>(lf)),
                         std::istreambuf_iterator<char>());

    // Log-ul trebuie sa contina cele 3 interogari
    ASSERT_TRUE(content.find("fox")         != std::string::npos);
    ASSERT_TRUE(content.find("programming") != std::string::npos);
    ASSERT_TRUE(content.find("unicorn")     != std::string::npos);

    // Curatam fisierele temporare
    fs::remove(logFile);
    removeTempDocs(dir);
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Suite 9: Index – director invalid ────────────────────────────────────────
// Verifica ca Index-ul nu crapa la un director inexistent.
// ─────────────────────────────────────────────────────────────────────────────
void test_index_invalid_directory() {
    std::cout << "\n[Suite 9] Index – director invalid\n";

    Index index;
    // Nu trebuie sa arunce exceptii, doar sa afiseze un mesaj de eroare
    index.loadFromDirectory("director_care_nu_exista_xyz_123");
    index.buildIndex();

    // Dupa incarcare dintr-un director invalid, indexul trebuie sa fie gol
    ASSERT_TRUE(index.search("orice").empty());
}

// ─────────────────────────────────────────────────────────────────────────────
// ── Punctul de intrare al testelor ───────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║         Motor de Cautare – Teste Unitare         ║\n"
              << "╚══════════════════════════════════════════════════╝\n";

    test_document_basic();
    test_document_observer();
    test_index_load_and_build();
    test_index_stop_words_removed();
    test_index_search_and();
    test_index_search_or();
    test_index_case_insensitive();
    test_logger_observer_integration();
    test_index_invalid_directory();

    // ── Sumar final ───────────────────────────────────────────────────────
    std::cout << "\n══════════════════════════════════════════════════\n"
              << "  Rezultate: "
              << testsPassed << " trecute, "
              << testsFailed << " esuate, "
              << testsRun    << " total\n"
              << "══════════════════════════════════════════════════\n";

    // Returnam cod de eroare diferit de 0 daca exista teste esuate
    return (testsFailed > 0) ? 1 : 0;
}
