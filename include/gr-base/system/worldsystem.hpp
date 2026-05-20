#pragma once

#include "gr-base/pool/poolObject.hpp"
#include "gr-base/system/world/chunk.hpp"

#include "gr-base/core/default.hpp"

#include <unordered_map>
#include <vector>
#include <string>

class WorldSystem
{
public:
    static WorldSystem& GetInstance();

    static ChunkID Create(const std::string& name);

    static ChunkID Find(const std::string& name);

    static StreamChunk* Get(ChunkID id);

    static void Update(Scene* scene);

    int CunkSize;

    int RenderDistance;

private:

    std::unordered_map<std::string, ChunkID> m_chunkNameMap;

    std::vector<ChunkID> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    WorldSystem();
    ~WorldSystem();

    ChunkID create(const std::string& name);

    ChunkID find(const std::string& name);

    StreamChunk* get(ChunkID id);

    void update(Scene* scene);

    void unloadChunk(ChunkID id);

    void render(StreamChunk* chunk);
};

