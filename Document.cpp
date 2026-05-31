#include "Document.h"
#include <iostream>
#include <algorithm>  
#include <cctype>     

Document::Document(const std::string& path, const std::string& text)
    : filePath(path), content(text) {}

void Document::addObserver(IObserver* obs) {
    observers_.push_back(obs);
}

void Document::removeObserver(IObserver* obs) {
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), obs),
        observers_.end());
}

void Document::notifyObservers(const std::string& query, int results) const {
    for (IObserver* obs : observers_)
        obs->onSearch(query, results);
}

#include <sstream>
#include <vector>

static std::string toLowerStr(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return res;
}

static bool containsWordIgnoreCase(const std::string& line, const std::string& word) {
    if (word.empty()) return false;
    std::string lowerLine = toLowerStr(line);
    std::string lowerWord = toLowerStr(word);
    return lowerLine.find(lowerWord) != std::string::npos;
}

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
        if (!preview.empty() && preview.back() == '\r') {
            preview.pop_back();
        }
        if (preview.size() > 120) {
            preview = preview.substr(0, 120) + "...";
        }
    }

    std::cout << "  Preview  : " << preview << "\n";
}
