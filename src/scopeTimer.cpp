#include "gr-base/scopeTimer.hpp"

void ScopeTimer::start()
{
    m_start = std::chrono::steady_clock::now();
}

double ScopeTimer::end() const
{
    auto end =
            std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration<double, std::milli>(
            end - m_start);

    return duration.count();
}



