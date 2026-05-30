#include "gr-base/system/worldsystem.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/transform.hpp"

extern "C"
{
#include "gr-base/externals/vector.h"
}

WorldSystem::WorldSystem()
{}

WorldSystem::~WorldSystem()
{}

void WorldSystem::Update(Scene* scene)
{
    const auto camera = scene->Wrap(scene->GetCamera());
    if (!camera.IsAlive())
        return;

    const auto transform = camera.GetComponent<Transform>();

    int currentX = floor(transform->position.x / CELL_SIZE);
    int currentZ = floor(transform->position.z / CELL_SIZE);

    std::unordered_set<CellID> m_chunkAvailable;

    for (int x=-CELL_RENDER_DISTANCE;x<CELL_RENDER_DISTANCE;x++)
    {
        for (int z=-CELL_RENDER_DISTANCE;z<CELL_RENDER_DISTANCE;z++)
        {
            int chunkIdX = currentX + x;
            int chunkIdZ = currentZ + z;

            std::string key = std::to_string(chunkIdX) + "_" + std::to_string(chunkIdZ);

            CellID chunkID = find(key);
            if (chunkID == INVALID_CELL)
            {
                chunkID = create(key);

                StreamChunk* chunk = get(chunkID);
                chunk->position = {(float)(chunkIdX * CELL_SIZE), (float)(chunkIdZ * CELL_SIZE)};
                chunk->entities = nullptr;
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

CellID WorldSystem::create(const std::string &name)
{
    CellID id = m_pool.allocate();

    StreamChunk* chunk = m_pool.get(id);
    chunk->name = strdup(name.c_str());

    m_chunkNameMap.emplace(name, id);

    m_chunkLoaded.push_back(id);

    return id;
}

CellID WorldSystem::find(const std::string &name)
{
    auto it = m_chunkNameMap.find(name);
    if (it == m_chunkNameMap.end())
        return INVALID_CELL;
    return it->second;
}

StreamChunk* WorldSystem::get(CellID id)
{
    return m_pool.get(id);
}

void WorldSystem::unloadChunk(CellID id)
{
    StreamChunk* chunk = get(id);

    if (chunk->entities != nullptr)
    {
        for (size_t i=0;i<vector_length(chunk->entities);i++)
        {
            // GameObjectHandle& handle = chunk->objects[i];

            // GameObject::Destroy(handle);
        }

        vector_free(chunk->entities);
    }

    free(chunk->name);

    m_pool.deallocate(id);
}

void WorldSystem::render(StreamChunk* chunk)
{
    Vector2 position = chunk->position;

    Vector3 p1{position.x,              0, position.y};
    Vector3 p2{position.x + CELL_SIZE, 0, position.y};
    Vector3 p3{position.x + CELL_SIZE, 0, position.y + CELL_SIZE};
    Vector3 p4{position.x,              0, position.y + CELL_SIZE};

    DebugDraw::DrawLine(p1, p2);
    DebugDraw::DrawLine(p2, p3);
    DebugDraw::DrawLine(p3, p4);
    DebugDraw::DrawLine(p4, p1);
}



