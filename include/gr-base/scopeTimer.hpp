#pragma once

#include <chrono>

class ScopeTimer
{
public:
    void start();

    double end() const;
    
private:
    std::chrono::high_resolution_clock::time_point m_start =
        std::chrono::high_resolution_clock::now();
};




