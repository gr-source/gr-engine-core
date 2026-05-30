#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/system/world/cell.hpp"

#include "gr-base/pool/poolObject.hpp"

#include "gr-base/core/default.hpp"

#include <unordered_map>
#include <vector>
#include <string>

class WorldSystem : public System
{
public:
    WorldSystem();
    ~WorldSystem() override;

    void Update(Scene* scene) override;

    int CunkSize;

    int RenderDistance;

private:
    std::unordered_map<std::string, CellID> m_chunkNameMap;

    std::vector<CellID> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    CellID create(const std::string& name);

    CellID find(const std::string& name);

    StreamChunk* get(CellID id);

    void unloadChunk(CellID id);

    void render(StreamChunk* chunk);
};

