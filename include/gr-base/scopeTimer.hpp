#pragma once

#include <chrono>

class ScopeTimer
{
public:
    void start();

    double end() const;
    
private:
    std::chrono::steady_clock::time_point m_start =
        std::chrono::steady_clock::now();
};




