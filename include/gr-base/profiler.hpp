#pragma once

#include <string>
#include <vector>

struct ProfileEntry
{
    std::string name;
    float timeMs;
};

class Profiler
{
public:
    static void BeginFrame();

    static void EndFrame();

    static void Add(const std::string& name, float ms);

    static std::vector<ProfileEntry> GetEntries();

private:
    static Profiler gProfiler;

    std::vector<ProfileEntry> entries[2];

    int readIndex = 0;
    int writeIndex = 1;

    Profiler();
    ~Profiler();

    void beginFrame();

    void endFrame();
};
