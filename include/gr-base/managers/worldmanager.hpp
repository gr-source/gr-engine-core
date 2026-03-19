#pragma once

#include "gr-base/core/chunk.hpp"

#include <unordered_map>

class WorldManager
{
public:
    static WorldManager& GetInstance();

    static ChunkID Create(const std::string& name);

    static ChunkID Find(const std::string& name);

    static StreamChunk* Get(ChunkID id);

    static void Update();

    int CunkSize;

    int RenderDistance;

private:

    std::unordered_map<std::string, ChunkID> m_chunkNameMap;

    std::vector<ChunkID> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    WorldManager();
    ~WorldManager();

    ChunkID create(const std::string& name);

    ChunkID find(const std::string& name);

    StreamChunk* get(ChunkID id);

    void update();

    void unloadChunk(ChunkID id);

    void render(StreamChunk* chunk);
};

