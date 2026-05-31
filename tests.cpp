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


static int testsRun    = 0;
static int testsPassed = 0;
static int testsFailed = 0;

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

static std::string createTempDocs() {
    std::string dir = "test_docs_tmp";
    fs::create_directories(dir);

    auto write = [&](const std::string& name, const std::string& content) {
        std::ofstream f(dir + "/" + name);
        f << content;
    };

    write("alpha.txt", "the quick brown fox jumps over the lazy dog");

    write("beta.txt",  "a fox in the forest hunts for rabbit");

    write("gamma.txt", "programming in cpp requires attention to detail");

    return dir;
}

static void removeTempDocs(const std::string& dir) {
    fs::remove_all(dir);
}

void test_document_basic() {
    std::cout << "\n[Suite 1] Document – atribute de baza\n";

    Document doc("cale/fisier.txt", "Salut lume");

    ASSERT_EQ(doc.filePath, std::string("cale/fisier.txt"));
    ASSERT_EQ(doc.content,  std::string("Salut lume"));
}

void test_document_observer() {
    std::cout << "\n[Suite 2] Document – Observer pattern\n";

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

    doc.addObserver(&obs);
    doc.notifyObservers("pisica", 3);

    ASSERT_EQ(obs.callCount,   1);
    ASSERT_EQ(obs.lastQuery,   std::string("pisica"));
    ASSERT_EQ(obs.lastResults, 3);

    doc.removeObserver(&obs);
    doc.notifyObservers("caine", 1);
    ASSERT_EQ(obs.callCount, 1);  

    MockObserver obs2;
    doc.addObserver(&obs);
    doc.addObserver(&obs2);
    doc.notifyObservers("test", 5);

    ASSERT_EQ(obs.callCount,  2);  
    ASSERT_EQ(obs2.callCount, 1);  
    ASSERT_EQ(obs2.lastResults, 5);
}

void test_index_load_and_build() {
    std::cout << "\n[Suite 3] Index – incarcare si construire index\n";

    std::string dir = createTempDocs();

    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    auto res = index.search("fox");
    ASSERT_EQ(res.size(), std::size_t(2));

    auto res2 = index.search("programming");
    ASSERT_EQ(res2.size(), std::size_t(1));

    auto res3 = index.search("unicorn");
    ASSERT_TRUE(res3.empty());

    removeTempDocs(dir);
}

void test_index_stop_words_removed() {
    std::cout << "\n[Suite 4] Index – eliminare stop-words\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    ASSERT_TRUE(index.search("the").empty());   
    ASSERT_TRUE(index.search("a").empty());     
    ASSERT_TRUE(index.search("in").empty());    
    ASSERT_TRUE(index.search("for").empty());   
    ASSERT_TRUE(index.search("to").empty());    

    removeTempDocs(dir);
}

void test_index_search_and() {
    std::cout << "\n[Suite 5] Index – cautare AND\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    auto res = index.search("fox forest", SearchMode::AND);
    ASSERT_EQ(res.size(), std::size_t(1));

    auto res2 = index.search("fox programming", SearchMode::AND);
    ASSERT_TRUE(res2.empty());

    auto res3 = index.search("fox quick", SearchMode::AND);
    ASSERT_EQ(res3.size(), std::size_t(1));

    auto res4 = index.search("fox dragon", SearchMode::AND);
    ASSERT_TRUE(res4.empty());

    removeTempDocs(dir);
}

void test_index_search_or() {
    std::cout << "\n[Suite 6] Index – cautare OR\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    auto res = index.search("fox programming", SearchMode::OR);
    ASSERT_EQ(res.size(), std::size_t(3));

    auto res2 = index.search("unicorn dragon", SearchMode::OR);
    ASSERT_TRUE(res2.empty());

    auto res3 = index.search("rabbit unicorn", SearchMode::OR);
    ASSERT_EQ(res3.size(), std::size_t(1)); 

    removeTempDocs(dir);
}

void test_index_case_insensitive() {
    std::cout << "\n[Suite 7] Index – cautare case-insensitive\n";

    std::string dir = createTempDocs();
    Index index;
    index.loadFromDirectory(dir);
    index.buildIndex();

    auto lower = index.search("fox");
    auto upper = index.search("FOX");
    auto mixed = index.search("FoX");

    ASSERT_EQ(lower.size(), upper.size());
    ASSERT_EQ(lower.size(), mixed.size());

    ASSERT_EQ(lower.size(), std::size_t(2));

    removeTempDocs(dir);
}

void test_logger_observer_integration() {
    std::cout << "\n[Suite 8] Logger – integrare Observer\n";

    std::string dir     = createTempDocs();
    std::string logFile = "test_search_log_tmp.txt";

    fs::remove(logFile);

    {
        Index  index;
        Logger logger(logFile);
        index.addObserver(&logger);
        index.loadFromDirectory(dir);
        index.buildIndex();

        index.search("fox");
        index.search("programming");
        index.search("unicorn");    
    } 

    ASSERT_TRUE(fs::exists(logFile));

    std::ifstream lf(logFile);
    std::string content((std::istreambuf_iterator<char>(lf)),
                         std::istreambuf_iterator<char>());

    ASSERT_TRUE(content.find("fox")         != std::string::npos);
    ASSERT_TRUE(content.find("programming") != std::string::npos);
    ASSERT_TRUE(content.find("unicorn")     != std::string::npos);

    fs::remove(logFile);
    removeTempDocs(dir);
}

void test_index_invalid_directory() {
    std::cout << "\n[Suite 9] Index – director invalid\n";

    Index index;
    index.loadFromDirectory("director_care_nu_exista_xyz_123");
    index.buildIndex();

    ASSERT_TRUE(index.search("orice").empty());
}

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

    std::cout << "\n══════════════════════════════════════════════════\n"
              << "  Rezultate: "
              << testsPassed << " trecute, "
              << testsFailed << " esuate, "
              << testsRun    << " total\n"
              << "══════════════════════════════════════════════════\n";

    return (testsFailed > 0) ? 1 : 0;
}
