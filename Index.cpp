#include "Index.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>  

namespace fs = std::filesystem;

const std::set<std::string> Index::STOP_WORDS = {

    "si", "in", "la", "de", "pe", "cu", "sa", "se", "ca", "nu",
    "un", "o",  "al", "ai", "ale", "lui", "ei", "lor", "sau",
    "dar", "ci", "daca", "mai", "tot", "cel", "cea", "cei", "cele",
    "este", "sunt", "era", "fost", "fi",
    "eu", "tu", "el", "ea", "noi", "voi",
    "ma", "te", "il", "le", "ne", "va",
    "din", "prin", "pentru", "spre", "despre", "intre",
    "astfel", "deci", "insa", "totusi", "chiar", "doar", "inca",
    "care", "cum", "cand", "unde", "cat",

    "the", "a",  "an", "and", "or", "but", "in", "on", "at",
    "to",  "of", "for", "with", "by", "from", "up", "is", "are",
    "was", "were", "be", "been", "has", "have", "had", "do", "does",
    "did", "not", "that", "this", "it", "its", "as", "if",
    "he",  "she", "we",  "they", "you", "i",
    "can", "will", "just", "about", "which", "so", "also"
};

Index::~Index() {
    for (Document* doc : documents_)
        delete doc;
    documents_.clear();
}

void Index::addObserver(IObserver* obs) {
    observers_.push_back(obs);
    for (Document* doc : documents_)
        doc->addObserver(obs);
}

void Index::removeObserver(IObserver* obs) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), obs),
        observers_.end());
    for (Document* doc : documents_)
        doc->removeObserver(obs);
}

void Index::notifyAll(const std::string& query, int results) const {
    if (!documents_.empty()) {
        documents_.front()->notifyObservers(query, results);
    } else {
        for (IObserver* obs : observers_)
            obs->onSearch(query, results);
    }
}

std::string Index::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::vector<std::string> Index::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string currentToken;

    auto addTokenIfValid = [&](const std::string& token) {
        if (token.empty()) return;
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
            currentToken += static_cast<char>(std::tolower(ch));
        } else {
            addTokenIfValid(currentToken);
            currentToken.clear();
        }
    }
    addTokenIfValid(currentToken);

    return tokens;
}

bool Index::isStopWord(const std::string& word) {
    return STOP_WORDS.count(word) > 0;
}

void Index::loadFromDirectory(const std::string& dirPath) {
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        std::cerr << "[Index] Eroare: '" << dirPath
                  << "' nu este un director valid.\n";
        return;
    }

    int loaded = 0;
    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".txt") continue;

        std::ifstream file(entry.path());
        if (!file.is_open()) {
            std::cerr << "[Index] Atentie: nu s-a putut deschide "
                      << entry.path() << "\n";
            continue;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        Document* doc = new Document(entry.path().string(), content);

        for (IObserver* obs : observers_)
            doc->addObserver(obs);

        documents_.push_back(doc);
        ++loaded;
    }

    std::cout << "[Index] Incarcate " << loaded
              << " fisier(e) din '" << dirPath << "'.\n";
}

void Index::buildIndex() {
    index_.clear();  

    int stopWordsRemoved = 0;
    int totalTokens      = 0;

    for (Document* doc : documents_) {
        std::vector<std::string> tokens = tokenize(doc->content);
        totalTokens += static_cast<int>(tokens.size());

        for (const std::string& word : tokens) {
            if (isStopWord(word)) {
                ++stopWordsRemoved;
                continue;
            }

            std::vector<Document*>& docList = index_[word];

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

std::vector<Document*> Index::search(const std::string& word) const {
    std::string normalised = toLower(word);
    std::vector<Document*> results;

    auto it = index_.find(normalised);
    if (it != index_.end())
        results = it->second;

    notifyAll(word, static_cast<int>(results.size()));
    return results;
}

std::vector<Document*> Index::search(const std::string& query, SearchMode mode) const {
    std::vector<std::string> words = tokenize(query);

    if (words.empty()) {
        notifyAll(query, 0);
        return {};
    }

    std::vector<Document*> results;

    if (mode == SearchMode::OR) {
        std::set<Document*> seen;  
        for (const std::string& w : words) {
            auto it = index_.find(w);
            if (it == index_.end()) continue;
            for (Document* doc : it->second) {
                if (seen.insert(doc).second)
                    results.push_back(doc);
            }
        }
    } else {
        auto it = index_.find(words[0]);
        if (it == index_.end()) {
            notifyAll(query, 0);
            return {};
        }
        results = it->second;

        for (std::size_t i = 1; i < words.size() && !results.empty(); ++i) {
            auto it2 = index_.find(words[i]);
            if (it2 == index_.end()) {
                results.clear();
                break;
            }
            const std::vector<Document*>& nextList = it2->second;

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

    notifyAll(query, static_cast<int>(results.size()));
    return results;
}

void Index::printIndex() const {
    std::cout << "\n=== Index Inversat (" << index_.size() << " cuvinte) ===\n";
    for (const auto& [word, docs] : index_)
        std::cout << "  " << word << " (" << docs.size() << " doc(uri))\n";
    std::cout << "==========================================\n\n";
}
