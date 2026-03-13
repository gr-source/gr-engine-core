#include "gr-base/managers/chunkmanager.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/task/runtime/chunkLoadTask.hpp"
#include "gr-base/core/task/runtime/chunkSaveTask.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-base/externals/array.h"
#include "gr-base/script/camera.hpp"
#include <functional>

ChunkID ChunkManager::Create(const std::string &name)
{
    return GetInstance().create(name);
}

ChunkID ChunkManager::Find(const std::string &name)
{
    return GetInstance().find(name);
}

StreamChunk* ChunkManager::Get(ChunkID id)
{
    return GetInstance().get(id);
}

void ChunkManager::Update()
{
    GetInstance().update();
}

// private
ChunkManager& ChunkManager::GetInstance()
{
    static ChunkManager s_ChunkManager;
    return s_ChunkManager;
}

ChunkManager::ChunkManager()
{}

ChunkManager::~ChunkManager()
{}

ChunkID ChunkManager::create(const std::string &name)
{
    ChunkID id = m_pool.allocate();

    StreamChunk* chunk = m_pool.get(id);
    chunk->name = strdup(name.c_str());

    m_chunkNameMap.emplace(name, id);

    m_chunkLoaded.push_back(id);

    return id;
}

ChunkID ChunkManager::find(const std::string &name)
{
    auto it = m_chunkNameMap.find(name);
    if (it == m_chunkNameMap.end())
        return INVALID_CHUNK;
    return it->second;
}

StreamChunk* ChunkManager::get(ChunkID id)
{
    return m_pool.get(id);
}

void ChunkManager::update()
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

                if (gfile::exists(path))
                    TaskManager::Add<int, ChunkLoadTask>(path, chunkID);
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
                TaskManager::Add<int, ChunkSaveTask>(*it, std::bind(&ChunkManager::unloadChunk, this, std::placeholders::_1));
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

void ChunkManager::unloadChunk(ChunkID id)
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

    m_pool.deallocte(id);
}

void ChunkManager::render(StreamChunk* chunk)
{
    Vector2 position = chunk->position;

    Vector3 p1{position.x, 0, position.y};
    Vector3 p2{position.x + CHUNK_SIZE, 0, position.y};
    Vector3 p3{position.x + CHUNK_SIZE, 0, position.y + CHUNK_SIZE};
    Vector3 p4{position.x, 0, position.y + CHUNK_SIZE};

    DebugDraw::DrawLine(p1, p2);
    DebugDraw::DrawLine(p2, p3);
    DebugDraw::DrawLine(p3, p4);
    DebugDraw::DrawLine(p4, p1);
}



