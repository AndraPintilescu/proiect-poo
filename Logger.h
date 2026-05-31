#pragma once
#include "IObserver.h"
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>


class Logger : public IObserver {
public:
    explicit Logger(const std::string& logFile = "search_log.txt");

    ~Logger() override;

    void onSearch(const std::string& query, int results) override;

private:
    std::ofstream logStream_;  
    
    static std::string timestamp();
};
