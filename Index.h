#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include "Document.h"
#include "IObserver.h"

enum class SearchMode {
    AND,    
    OR      
};

class Index {
public:
    Index() = default;
    ~Index();   

    void addObserver(IObserver* obs);

    void removeObserver(IObserver* obs);

    void loadFromDirectory(const std::string& dirPath);

    void buildIndex();

    std::vector<Document*> search(const std::string& word) const;

    std::vector<Document*> search(const std::string& query, SearchMode mode) const;

    void printIndex() const;

private:
    std::vector<Document*>                      documents_; 
    std::map<std::string, std::vector<Document*>> index_;    
    std::vector<IObserver*>                     observers_;  

    static const std::set<std::string> STOP_WORDS;

    static std::string toLower(const std::string& s);

    static std::vector<std::string> tokenize(const std::string& text);

    static bool isStopWord(const std::string& word);

    void notifyAll(const std::string& query, int results) const;
};
