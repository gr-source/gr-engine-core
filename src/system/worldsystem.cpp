#include "gr-base/system/worldsystem.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/transform.hpp"

extern "C"
{
#include "gr-base/externals/vector.h"
}

ChunkID WorldSystem::Create(const std::string &name)
{
    return GetInstance().create(name);
}

ChunkID WorldSystem::Find(const std::string &name)
{
    return GetInstance().find(name);
}

StreamChunk* WorldSystem::Get(ChunkID id)
{
    return GetInstance().get(id);
}

void WorldSystem::Update(Scene* scene)
{
    GetInstance().update(scene);
}

// private
WorldSystem& WorldSystem::GetInstance()
{
    static WorldSystem s_WorldSystem;
    return s_WorldSystem;
}

WorldSystem::WorldSystem()
{}

WorldSystem::~WorldSystem()
{}

ChunkID WorldSystem::create(const std::string &name)
{
    ChunkID id = m_pool.allocate();

    StreamChunk* chunk = m_pool.get(id);
    chunk->name = strdup(name.c_str());

    m_chunkNameMap.emplace(name, id);

    m_chunkLoaded.push_back(id);

    return id;
}

ChunkID WorldSystem::find(const std::string &name)
{
    auto it = m_chunkNameMap.find(name);
    if (it == m_chunkNameMap.end())
        return INVALID_CHUNK;
    return it->second;
}

StreamChunk* WorldSystem::get(ChunkID id)
{
    return m_pool.get(id);
}

void WorldSystem::update(Scene* scene)
{
    const auto camera = scene->Wrap(scene->GetCamera());
    if (!camera.IsAlive())
        return;

    const auto transform = camera.GetComponent<Transform>();

    int currentX = floor(transform->position.x / CHUNK_SIZEI);
    int currentZ = floor(transform->position.z / CHUNK_SIZEI);

    std::unordered_set<ChunkID> m_chunkAvailable;

    for (int x=-CHUNK_RENDER_DISTANCE;x<CHUNK_RENDER_DISTANCE;x++)
    {
        for (int z=-CHUNK_RENDER_DISTANCE;z<CHUNK_RENDER_DISTANCE;z++)
        {
            int chunkIdX = currentX + x;
            int chunkIdZ = currentZ + z;

            std::string key = std::to_string(chunkIdX) + "_" + std::to_string(chunkIdZ);

            ChunkID chunkID = find(key);
            if (chunkID == INVALID_CHUNK)
            {
                chunkID = create(key);

                StreamChunk* chunk = get(chunkID);
                chunk->position = {(float)(chunkIdX * CHUNK_SIZEI), (float)(chunkIdZ * CHUNK_SIZEI)};
                chunk->objects = nullptr;
                chunk->dirty = false;
            }

            m_chunkAvailable.emplace(chunkID);
        }
    }

    for (auto it = m_chunkLoaded.begin(); it != m_chunkLoaded.end(); )
    {
        if (m_chunkAvailable.find(*it) == m_chunkAvailable.end())
        {
            StreamChunk* chunk = m_pool.get(*it);

            m_chunkNameMap.erase(chunk->name);

            unloadChunk(*it);

            it = m_chunkLoaded.erase(it);
        } else
        {
            ++it;
        }
    }

    for (auto& chunkID : m_chunkLoaded)
        render(get(chunkID));
}

void WorldSystem::unloadChunk(ChunkID id)
{
    StreamChunk* chunk = get(id);

    if (chunk->objects != nullptr)
    {
        for (size_t i=0;i<vector_length(chunk->objects);i++)
        {
            // GameObjectHandle& handle = chunk->objects[i];

            // GameObject::Destroy(handle);
        }

        vector_free(chunk->objects);
    }

    free(chunk->name);

    m_pool.deallocate(id);
}

void WorldSystem::render(StreamChunk* chunk)
{
    Vector2 position = chunk->position;

    Vector3 p1{position.x,              0, position.y};
    Vector3 p2{position.x + CHUNK_SIZEI, 0, position.y};
    Vector3 p3{position.x + CHUNK_SIZEI, 0, position.y + CHUNK_SIZEI};
    Vector3 p4{position.x,              0, position.y + CHUNK_SIZEI};

    DebugDraw::DrawLine(p1, p2);
    DebugDraw::DrawLine(p2, p3);
    DebugDraw::DrawLine(p3, p4);
    DebugDraw::DrawLine(p4, p1);
}



