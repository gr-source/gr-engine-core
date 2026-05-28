#pragma once

#include "gr-base/system/world/cell.hpp"

#include "gr-base/pool/poolObject.hpp"

#include "gr-base/core/default.hpp"

#include <unordered_map>
#include <vector>
#include <string>

class WorldSystem
{
public:
    static void Update(Scene* scene);

    int CunkSize;

    int RenderDistance;

private:
    static WorldSystem& GetInstance();

    std::unordered_map<std::string, CellID> m_chunkNameMap;

    std::vector<CellID> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    WorldSystem();
    ~WorldSystem();

    CellID create(const std::string& name);

    CellID find(const std::string& name);

    StreamChunk* get(CellID id);

    void update(Scene* scene);

    void unloadChunk(CellID id);

    void render(StreamChunk* chunk);
};

