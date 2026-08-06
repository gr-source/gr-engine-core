#include "gr-base/profiler.hpp"

Profiler Profiler::gProfiler;

//public
void Profiler::BeginFrame()
{
    gProfiler.beginFrame();
}

void Profiler::EndFrame()
{
    gProfiler.endFrame();
}

void Profiler::Add(const std::string& name, float ms)
{
    gProfiler.entries[gProfiler.writeIndex].push_back({name, ms});
}

std::vector<ProfileEntry> Profiler::GetEntries()
{
    return gProfiler.entries[gProfiler.readIndex];
}

// private
Profiler::Profiler()
{}

Profiler::~Profiler()
{}

void Profiler::beginFrame()
{
}

void Profiler::endFrame()
{
    std::swap(readIndex, writeIndex);

    entries[writeIndex].clear();
}

