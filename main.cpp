// ── main.cpp ──────────────────────────────────────────────────────────────────
// Punctul de intrare al motorului de cautare.
//
// Demonstreaza:
//   1. Initializarea Index-ului si a Logger-ului (Observer pattern)
//   2. Incarcarea documentelor din folderul "docs/"
//   3. Construirea indexului inversat (cu eliminare stop-words)
//   4. Cautari automate de demonstratie (simpla, AND, OR)
//   5. Bucla interactiva de cautare cu comenzi AND / OR
//
// Compilare (MSVC – Visual Studio): setati Standard C++17 in Project Properties
// Compilare (g++):
//   g++ -std=c++17 -o search_engine main.cpp Document.cpp Index.cpp Logger.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include <iostream>
#include <string>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Index.h"
#include "Logger.h"

// ── Functie auxiliara: afisare rezultate ─────────────────────────────────────
// Afiseaza documentele gasite intr-un format clar, sau un mesaj daca nu s-a
// gasit nimic.
static void printResults(const std::vector<Document*>& results,
                         const std::string& query) {
    if (results.empty()) {
        std::cout << "  [!] Niciun document gasit pentru \""
                  << query << "\".\n\n";
    } else {
        std::cout << "  [*] Gasite " << results.size()
                  << " document(e) pentru \"" << query << "\":\n";
        for (Document* doc : results)
            doc->print(query);
        std::cout << "\n";
    }
}

// ── Functie auxiliara: afisare meniu de ajutor ───────────────────────────────
static void showHelp() {
    std::cout << "\n"
              << "  Comenzi disponibile:\n"
              << "  ─────────────────────────────────────────────\n"
              << "    <cuvant>              – cautare simpla\n"
              << "    AND <cuv1> <cuv2> ... – documente cu TOATE cuvintele\n"
              << "    OR  <cuv1> <cuv2> ... – documente cu ORICARE cuvant\n"
              << "    index                 – afiseaza indexul complet\n"
              << "    help                  – afiseaza acest ajutor\n"
              << "    quit / exit           – inchide programul\n"
              << "  ─────────────────────────────────────────────\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// ── main ──────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║       Motor de Cautare pentru Documente Text     ║\n"
              << "║           Proiect POO – C++17                    ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    // ── 1. Initializare Index ─────────────────────────────────────────────
    Index index;

    // ── 2. Atasare Logger (Observer pattern) ──────────────────────────────
    // Logger-ul este creat INAINTE de loadFromDirectory astfel incat
    // sa primeasca notificari pentru toate cautarile.
    Logger logger("search_log.txt");
    index.addObserver(&logger);
    std::cout << "[Main] Logger atasat. Cautarile vor fi salvate in 'search_log.txt'.\n\n";

    // ── 3. Incarcare documente din folderul "docs/" ───────────────────────
    const std::string docsPath = "docs";
    index.loadFromDirectory(docsPath);

    // ── 4. Construire index inversat ──────────────────────────────────────
    index.buildIndex();

    // ── 5. Cautari automate de demonstratie ───────────────────────────────
    std::cout << "\n╔══════════════════════════════════════════════════╗\n"
              << "║           DEMONSTRATIE CAUTARI AUTOMATE          ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    // 5a. Cautare simpla (un singur cuvant)
    std::cout << ">> Cautare simpla: \"vulpea\"\n";
    printResults(index.search("vulpea"), "vulpea");

    // 5b. Cautare AND (intersectie) – ambele cuvinte trebuie sa apara
    std::cout << ">> Cautare AND: \"calculator programarea\"\n";
    printResults(index.search("calculator programarea", SearchMode::AND),
                 "calculator AND programarea");

    // 5c. Cautare OR (reuniune) – cel putin un cuvant trebuie sa apara
    std::cout << ">> Cautare OR: \"vulpea lupul\"\n";
    printResults(index.search("vulpea lupul", SearchMode::OR),
                 "vulpea OR lupul");

    // 5d. Cautare care nu gaseste nimic
    std::cout << ">> Cautare simpla (inexistent): \"unicorn\"\n";
    printResults(index.search("unicorn"), "unicorn");

    // 5e. Demonstrarea case-insensitive
    std::cout << ">> Cautare case-insensitive: \"MOTORUL\" vs \"motorul\"\n";
    auto r1 = index.search("MOTORUL");
    auto r2 = index.search("motorul");
    std::cout << "  \"MOTORUL\" => " << r1.size() << " doc(uri)\n";
    std::cout << "  \"motorul\" => " << r2.size() << " doc(uri)\n\n";

    // ── 6. Bucla interactiva ──────────────────────────────────────────────
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║             MOTOR DE CAUTARE INTERACTIV          ║\n"
              << "╚══════════════════════════════════════════════════╝\n";
    showHelp();

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line))
            break;                          // EOF (Ctrl+Z / Ctrl+D)

        // Ignoram liniile goale
        if (line.empty()) continue;

        // Comenzi speciale
        if (line == "quit" || line == "exit") break;
        if (line == "help")  { showHelp();           continue; }
        if (line == "index") { index.printIndex();   continue; }

        // Parsam prefixul AND / OR
        std::istringstream iss(line);
        std::string first;
        iss >> first;

        if (first == "AND" || first == "OR") {
            // Restul liniei dupa prefix reprezinta cuvintele de cautat
            std::string rest;
            std::getline(iss, rest);
            // Eliminam spatiul de la inceput
            if (!rest.empty() && rest[0] == ' ')
                rest = rest.substr(1);

            if (rest.empty()) {
                std::cout << "  [!] Specificati cel putin un cuvant dupa "
                          << first << ".\n\n";
                continue;
            }

            SearchMode mode = (first == "AND") ? SearchMode::AND : SearchMode::OR;
            printResults(index.search(rest, mode), first + " " + rest);
        } else {
            // Cautare simpla cu primul token din linie
            printResults(index.search(first), first);
        }
    }

    std::cout << "\nLa revedere!\n";
    return 0;
}
