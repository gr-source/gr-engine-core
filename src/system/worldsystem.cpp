#include "gr-base/system/worldsystem.hpp"

#include "gr-base/debug/debugDraw.hpp"

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

    for (int x=-LOAD_DISTANCE;x<LOAD_DISTANCE;x++)
    {
        for (int z=-LOAD_DISTANCE;z<LOAD_DISTANCE;z++)
        {
            int chunkIdX = currentX + x;
            int chunkIdZ = currentZ + z;

            ChunkCoord coord{chunkIdX, chunkIdZ};

            if (!has(coord))
                create(coord);
        }
    }

    for (auto it = m_chunkLoaded.begin(); it != m_chunkLoaded.end(); )
    {
        auto chunk = m_pool.get(*it);

        int dx = chunk->coord.x - currentX;
        int dz = chunk->coord.z - currentZ;

        if(std::abs(dx) > LOAD_DISTANCE ||
           std::abs(dz) > LOAD_DISTANCE)
        {
            m_chunkNameMap.erase(chunk->coord);

            unloadChunk(*it);

            it = m_chunkLoaded.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto& chunkID : m_chunkLoaded)
        render(m_pool.get(chunkID));
}

void WorldSystem::create(const ChunkCoord& coord)
{
    CellID id = m_pool.allocate();

    StreamChunk* chunk = m_pool.get(id);
    chunk->coord = coord;
    chunk->position = {(float)(coord.x * CELL_SIZE), (float)(coord.z * CELL_SIZE)};
    chunk->entities = nullptr;
    chunk->state = ChunkState::Loaded;

    m_chunkNameMap.emplace(coord, id);

    m_chunkLoaded.push_back(id);
}

bool WorldSystem::has(const ChunkCoord& coord) const
{
    return m_chunkNameMap.find(coord) != m_chunkNameMap.end();
}

void WorldSystem::unloadChunk(CellID id)
{
    StreamChunk* chunk = m_pool.get(id);

    if (chunk->entities != nullptr)
    {
        for (size_t i=0;i<vector_length(chunk->entities);i++)
        {
            // GameObjectHandle& handle = chunk->objects[i];

            // GameObject::Destroy(handle);
        }

        vector_free(chunk->entities);
    }

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



