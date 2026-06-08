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
    std::unordered_map<ChunkCoord, CellID, ChunkCoordHash> m_chunkNameMap;

    std::vector<CellID> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    void create(const ChunkCoord& coord);

    bool has(const ChunkCoord& coord) const;

    void unloadChunk(CellID id);

    void render(StreamChunk* chunk);
};

