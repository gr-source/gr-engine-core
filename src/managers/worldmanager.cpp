#include "gr-base/managers/worldmanager.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/script/camera.hpp"

ChunkID WorldManager::Create(const std::string &name)
{
    return GetInstance().create(name);
}

ChunkID WorldManager::Find(const std::string &name)
{
    return GetInstance().find(name);
}

StreamChunk* WorldManager::Get(ChunkID id)
{
    return GetInstance().get(id);
}

void WorldManager::Update()
{
    GetInstance().update();
}

// private
WorldManager& WorldManager::GetInstance()
{
    static WorldManager s_WorldManager;
    return s_WorldManager;
}

WorldManager::WorldManager()
{}

WorldManager::~WorldManager()
{}

ChunkID WorldManager::create(const std::string &name)
{
    ChunkID id = m_pool.allocate();

    StreamChunk* chunk = m_pool.get(id);
    chunk->name = strdup(name.c_str());

    m_chunkNameMap.emplace(name, id);

    m_chunkLoaded.push_back(id);

    return id;
}

ChunkID WorldManager::find(const std::string &name)
{
    auto it = m_chunkNameMap.find(name);
    if (it == m_chunkNameMap.end())
        return INVALID_CHUNK;
    return it->second;
}

StreamChunk* WorldManager::get(ChunkID id)
{
    return m_pool.get(id);
}

void WorldManager::update()
{
    auto scene = Scene::GetCurrent();
    if (scene == nullptr)
        return;

    auto camera = scene->getCamera();
    if (camera == nullptr)
        return;

    auto position = camera->getPosition();

    int currentX = floor(position.x / CHUNK_SIZE);
    int currentZ = floor(position.z / CHUNK_SIZE);

    std::unordered_set<ChunkID> m_chunkAvailable;

    std::string dir = "/home/NoirVoid/gr-engine/";

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
                chunk->position = {(float)(chunkIdX * CHUNK_SIZE), (float)(chunkIdZ * CHUNK_SIZE)};
                chunk->objects = nullptr;
                chunk->dirty = false;

                std::string path = dir + key + ".grck";
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

            if (chunk->dirty)
            {
                chunk->dirty = false;
            } else
            {
                unloadChunk(*it);
            }

            it = m_chunkLoaded.erase(it);
        } else
        {
            ++it;
        }
    }

    for (auto& chunkID : m_chunkLoaded)
        render(get(chunkID));
}

void WorldManager::unloadChunk(ChunkID id)
{
    StreamChunk* chunk = get(id);

    if (chunk->objects != nullptr)
    {
        for (size_t i=0;i<vector_length(chunk->objects);i++)
        {
            GameObjectHandle& handle = chunk->objects[i];

            GameObject::Destroy(handle);
        }

        vector_free(chunk->objects);
    }

    free(chunk->name);

    m_pool.deallocate(id);
}

void WorldManager::render(StreamChunk* chunk)
{
    Vector2 position = chunk->position;

    Vector3 p1{position.x,              0, position.y};
    Vector3 p2{position.x + CHUNK_SIZE, 0, position.y};
    Vector3 p3{position.x + CHUNK_SIZE, 0, position.y + CHUNK_SIZE};
    Vector3 p4{position.x,              0, position.y + CHUNK_SIZE};

    DebugDraw::DrawLine(p1, p2);
    DebugDraw::DrawLine(p2, p3);
    DebugDraw::DrawLine(p3, p4);
    DebugDraw::DrawLine(p4, p1);
}



