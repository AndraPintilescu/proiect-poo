#pragma once
#include <string>
#include <vector>
#include "IObserver.h"

class Document {
public:
    std::string filePath;  
    std::string content;   

    Document(const std::string& path, const std::string& text);

    void addObserver(IObserver* obs);

    void removeObserver(IObserver* obs);

    void notifyObservers(const std::string& query, int results) const;

    void print(const std::string& query = "") const;

private:
    std::vector<IObserver*> observers_;
};
