#pragma once
#include <string>


class IObserver {
public:
    virtual ~IObserver() = default;

    virtual void onSearch(const std::string& query, int results) = 0;
};
