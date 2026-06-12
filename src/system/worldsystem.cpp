#include "gr-base/system/worldsystem.hpp"

#include "gr-base/core/renderer.hpp"
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

    int currentX = floor(transform->position.x / TERRAIN_CHUNK_SIZE);
    int currentZ = floor(transform->position.z / TERRAIN_CHUNK_SIZE);

    for (int x=-LOAD_DISTANCE;x<=LOAD_DISTANCE;x++)
    {
        for (int z=-LOAD_DISTANCE;z<=LOAD_DISTANCE;z++)
        {
            int chunkIdX = currentX + x;
            int chunkIdZ = currentZ + z;

            ChunkCoord coord{chunkIdX, chunkIdZ};

            ChunkCoordHash s_ChunkCoordHash;

            if (!has(s_ChunkCoordHash(coord)))
                create(coord);
        }
    }

    for (auto it = m_chunkLoaded.begin(); it != m_chunkLoaded.end(); )
    {
        auto chunk =& m_chunkNameMap[*it];

        int dx = chunk->coord.x - currentX;
        int dz = chunk->coord.z - currentZ;

        if(std::abs(dx) > LOAD_DISTANCE ||
           std::abs(dz) > LOAD_DISTANCE)
        {
            unloadChunk(*it);


            m_chunkNameMap.erase(*it);

            it = m_chunkLoaded.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto& hash : m_chunkLoaded)
        render(&m_chunkNameMap[hash]);
}

#include "gr-base/managers/assetsManager.hpp"

struct MyVertex3D
{
    Vector3 position;
    Vector3 normal;
    Vector2 texCoords;
};

MeshHandle WorldSystem::CreateMesh(const ChunkCoord& coord)
{
    ChunkCoordHash hash;

    MeshHandle handle = AssetsManager::CreateMesh(hash(coord));
    Mesh* mesh = AssetsManager::GetMesh(handle);

    float spacing = 1.0f;

    std::vector<MyVertex3D> vertices;
    std::vector<uint32_t> indices;

    constexpr float SIN_AMPLITUDE = 8.0f;
    constexpr float COS_AMPLITUDE = 6.0f;

    constexpr float TERRAIN_HEIGHT = 10.0f;

    // Vértices
    for (int x = 0; x <= TERRAIN_CHUNK_SIZE; x++)
    {
        for (int z = 0; z <= TERRAIN_CHUNK_SIZE; z++)
        {
            float localX = (float)x / (TERRAIN_CHUNK_SIZE - 1);
            float localZ = (float)z / (TERRAIN_CHUNK_SIZE - 1);

            int worldX = coord.x * TERRAIN_CHUNK_SIZE + x;
            int worldZ = coord.z * TERRAIN_CHUNK_SIZE + z;

            float height =
                sin(worldX * 0.05f) * SIN_AMPLITUDE +
                cos(worldZ * 0.04f) * COS_AMPLITUDE;

            float minHeight = -(SIN_AMPLITUDE + COS_AMPLITUDE);
            float maxHeight =  (SIN_AMPLITUDE + COS_AMPLITUDE);

            float normalizedHeight =
                (height - minHeight) / (maxHeight - minHeight);

            MyVertex3D vertex{};
            vertex.position = {
                localX,
                normalizedHeight * TERRAIN_HEIGHT,
                localZ
            };

            vertex.normal = { 0.0f, 1.0f, 0.0f };

            vertex.texCoords = {
                (float)(x + TERRAIN_CHUNK_SIZE) / TERRAIN_CHUNK_SIZE,
                (float)(z + TERRAIN_CHUNK_SIZE) / TERRAIN_CHUNK_SIZE
            };

            vertices.push_back(vertex);
        }
    }

    for (int x = 0; x < TERRAIN_CHUNK_SIZE; x++)
    {
        for (int z = 0; z < TERRAIN_CHUNK_SIZE; z++)
        {
            int topLeft     = x * (TERRAIN_CHUNK_SIZE + 1) + z;
            int topRight    = topLeft + 1;
            int bottomLeft  = (x + 1) * (TERRAIN_CHUNK_SIZE + 1) + z;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);

            indices.push_back(topRight);
            indices.push_back(bottomRight);
            indices.push_back(bottomLeft);
        }
    }

    for (uint32_t i = 0; i < indices.size(); i += 3)
    {
        const auto kIdx1 = indices[i + 0];
        const auto kIdx2 = indices[i + 1];
        const auto kIdx3 = indices[i + 2];

        auto& v0 = vertices[kIdx1];
        auto& v1 = vertices[kIdx2];
        auto& v2 = vertices[kIdx3];

        const Vector3 kNormal = Math::normalize(
            Math::cross(v1.position - v0.position, v2.position - v0.position)
        );

        // Save for each vertex
        v0.normal += kNormal;
        v1.normal += kNormal;
        v2.normal += kNormal;
    }

    // Normalize each vertex normal
    for (auto& v : vertices)
        v.normal = Math::normalize(v.normal);

    std::vector<Vertex3D> openglvertices;
    for (auto& v : vertices)
        openglvertices.push_back({v.position, v.normal, v.texCoords});

    mesh->vertices = (Vertex3D*)malloc(openglvertices.size() * sizeof(Vertex3D));
    mesh->numVertices = openglvertices.size();

    memcpy(mesh->vertices, openglvertices.data(), openglvertices.size() * sizeof(Vertex3D));

    Submesh submesh;
    submesh.indices = (uint32_t*)malloc(indices.size() * sizeof(uint32_t));
    submesh.numIndices = indices.size();

    memcpy(submesh.indices, indices.data(), indices.size() * sizeof(uint32_t));

    std::vector<Submesh> submeshes;
    submeshes.push_back(submesh);

    mesh->submeshes = (Submesh*)malloc(submeshes.size() * sizeof(Submesh));
    mesh->numSubmeshes = submeshes.size();

    memcpy(mesh->submeshes, submeshes.data(), submeshes.size() * sizeof(Submesh));

    return handle;
}

void WorldSystem::create(const ChunkCoord& coord)
{
    // CellID id = m_pool.allocate();

    StreamChunk chunk;
    chunk.coord = coord;
    chunk.position = {
        (float)coord.x * TERRAIN_CHUNK_SIZE,
        (float)coord.z * TERRAIN_CHUNK_SIZE
    };

    chunk.entities = nullptr;
    chunk.state = ChunkState::Loaded;
    chunk.transform = Math::CreateTRS(
            Vector3{ chunk.position.x, 0.0f, chunk.position.y },
            Quaternion::identity,
            Vector3{ float(TERRAIN_CHUNK_SIZE), 1.0f, float(TERRAIN_CHUNK_SIZE) }
        );

    ChunkCoordHash s_ChunkCoordHash;

    Hash64 hash = s_ChunkCoordHash(coord);

    chunk.mesh = AssetsManager::FindMesh(hash);
    if (chunk.mesh == HANDLE_INVALID)
        chunk.mesh = CreateMesh(coord);

    chunk.material = AssetsManager::FindMaterial(engine::hash::fnv1_str("material/DefaultMaterial"));
    if (chunk.material == HANDLE_INVALID)
        chunk.material = AssetsManager::LoadMaterial(engine::hash::fnv1_str("material/DefaultMaterial"));

    m_chunkNameMap.emplace(hash, chunk);

    m_chunkLoaded.push_back(hash);
}

bool WorldSystem::has(Hash64 hash) const
{
    return m_chunkNameMap.find(hash) != m_chunkNameMap.end();
}

void WorldSystem::unloadChunk(Hash64 hash)
{
    StreamChunk* chunk = &m_chunkNameMap[hash];

    if (chunk->entities != nullptr)
    {
        for (size_t i=0;i<vector_length(chunk->entities);i++)
        {
            // GameObjectHandle& handle = chunk->objects[i];

            // GameObject::Destroy(handle);
        }

        vector_free(chunk->entities);
    }

    if (chunk->mesh != HANDLE_INVALID)
    {
        auto mesh = AssetsManager::GetMesh(chunk->mesh);
        if (mesh != nullptr)
        {
            free(mesh->vertices);
            for (uint32_t i=0;i<mesh->numSubmeshes;i++)
            {
                auto& submesh = mesh->submeshes[i];
                free(submesh.indices);
            }
            free(mesh->submeshes);

            if (mesh->gpuMesh.pbo != nullptr)
                delete mesh->gpuMesh.pbo;

            if (mesh->gpuMesh.vertexarray != nullptr)
                delete mesh->gpuMesh.vertexarray;

            AssetsManager::RemoveMesh(chunk->mesh);
        }
    }

    // m_pool.deallocate(id);
}

void WorldSystem::render(StreamChunk* chunk)
{
    Vector2 position = chunk->position;

    Vector3 p1{position.x,              0, position.y};
    Vector3 p2{position.x + TERRAIN_CHUNK_SIZE, 0, position.y};
    Vector3 p3{position.x + TERRAIN_CHUNK_SIZE, 0, position.y + TERRAIN_CHUNK_SIZE};
    Vector3 p4{position.x,              0, position.y + TERRAIN_CHUNK_SIZE};

    DebugDraw::DrawLine(p1, p2);
    DebugDraw::DrawLine(p2, p3);
    DebugDraw::DrawLine(p3, p4);
    DebugDraw::DrawLine(p4, p1);

    Renderer::Submit(chunk->material, chunk->mesh, 0, &chunk->transform);
    // Renderer::Submit(chunk->material, chunk->mesh, 0, &Matrix4x4::identityMatrix);
}



