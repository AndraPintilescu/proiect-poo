#include <iostream>
#include <string>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "Index.h"
#include "Logger.h"

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

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║       Motor de Cautare pentru Documente Text     ║\n"
              << "║           Proiect POO – C++17                    ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    Index index;

    Logger logger("search_log.txt");
    index.addObserver(&logger);
    std::cout << "[Main] Logger atasat. Cautarile vor fi salvate in 'search_log.txt'.\n\n";

    const std::string docsPath = "docs";
    index.loadFromDirectory(docsPath);

    index.buildIndex();

    std::cout << "\n╔══════════════════════════════════════════════════╗\n"
              << "║           DEMONSTRATIE CAUTARI AUTOMATE          ║\n"
              << "╚══════════════════════════════════════════════════╝\n\n";

    std::cout << ">> Cautare simpla: \"vulpea\"\n";
    printResults(index.search("vulpea"), "vulpea");

    std::cout << ">> Cautare AND: \"calculator programarea\"\n";
    printResults(index.search("calculator programarea", SearchMode::AND),
                 "calculator AND programarea");

    std::cout << ">> Cautare OR: \"vulpea lupul\"\n";
    printResults(index.search("vulpea lupul", SearchMode::OR),
                 "vulpea OR lupul");

    std::cout << ">> Cautare simpla (inexistent): \"unicorn\"\n";
    printResults(index.search("unicorn"), "unicorn");

    std::cout << ">> Cautare case-insensitive: \"MOTORUL\" vs \"motorul\"\n";
    auto r1 = index.search("MOTORUL");
    auto r2 = index.search("motorul");
    std::cout << "  \"MOTORUL\" => " << r1.size() << " doc(uri)\n";
    std::cout << "  \"motorul\" => " << r2.size() << " doc(uri)\n\n";

    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║             MOTOR DE CAUTARE INTERACTIV          ║\n"
              << "╚══════════════════════════════════════════════════╝\n";
    showHelp();

    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line))
            break;                         

        if (line.empty()) continue;

        if (line == "quit" || line == "exit") break;
        if (line == "help")  { showHelp();           continue; }
        if (line == "index") { index.printIndex();   continue; }

        std::istringstream iss(line);
        std::string first;
        iss >> first;

        if (first == "AND" || first == "OR") {
            std::string rest;
            std::getline(iss, rest);
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
            printResults(index.search(first), first);
        }
    }

    std::cout << "\nLa revedere!\n";
    return 0;
}
