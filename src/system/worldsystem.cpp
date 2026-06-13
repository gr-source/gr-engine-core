#include "gr-base/system/worldsystem.hpp"

#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/renderer.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/debug/debugDraw.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/system/world/cell.hpp"

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

    for (int x=-TERRAIN_LOAD_DISTANCE;x<=TERRAIN_LOAD_DISTANCE;x++)
    {
        for (int z=-TERRAIN_LOAD_DISTANCE;z<=TERRAIN_LOAD_DISTANCE;z++)
        {
            int chunkIdX = currentX + x;
            int chunkIdZ = currentZ + z;

            // ChunkCoord coord{chunkIdX, chunkIdZ};

            Handle hash = handle_create(chunkIdX, chunkIdZ);

            // ChunkCoordHash s_ChunkCoordHash;

            if (!has(hash))
                create(hash);
        }
    }

    for (auto it = m_chunkLoaded.begin(); it != m_chunkLoaded.end(); )
    {
        auto chunk = m_pool.get(*it);

        int dx = ((int)handle_get_first(chunk->hash)) - currentX;
        int dz = ((int)handle_get_second(chunk->hash)) - currentZ;

        int distance = std::max(std::abs(dx), std::abs(dz));

        if (distance > TERRAIN_UNLOAD_DISTANCE)
        {
            if (chunk->state != ChunkState::LOADING)
            {
                m_chunkNameMap.erase(chunk->hash);

                unloadChunk(*it);

                it = m_chunkLoaded.erase(it);
            } else
            {
                ++it;
            }
        }
        else
        {
            if (distance > TERRAIN_LOAD_DISTANCE)
            {
                if (chunk->state == ChunkState::LOADED)
                    chunk->state = ChunkState::INACTIVE;
            } else
            {
                if (chunk->state == ChunkState::INACTIVE)
                    chunk->state = ChunkState::LOADED;
            }

            ++it;
        }
    }

    StreamChunk* chunks = m_pool.getRaw();
    for (size_t i=0;i<m_pool.getCount();i++)
        render(chunks + i, transform->position);
}

#include "gr-base/managers/assetsManager.hpp"

void WorldSystem::create(Hash64 hash)
{
    Handle h = m_pool.create();

    StreamChunk* chunk = m_pool.get(h);
    chunk->hash = hash;
    Vector3 position = {
        (float)((int)handle_get_first(hash)) * TERRAIN_CHUNK_SIZE,
        0.0f,
        (float)((int)handle_get_second(hash)) * TERRAIN_CHUNK_SIZE
    };

    chunk->state = ChunkState::LOADING;
    chunk->transform = Math::CreateTRS(
            position,
            Quaternion::identity,
            Vector3{ float(TERRAIN_CHUNK_SIZE), 1.0f, float(TERRAIN_CHUNK_SIZE) }
        );

    Threadpool::AddTask(&WorldSystem::buildMesh, this, hash, h);

    chunk->material = AssetsManager::FindMaterial(engine::hash::fnv1_str("material/Tree.Birch.Leaf.Summer.Mat"));
    if (chunk->material == HANDLE_INVALID)
        chunk->material = AssetsManager::LoadMaterial(engine::hash::fnv1_str("material/Tree.Birch.Leaf.Summer.Mat"));

    m_chunkNameMap.emplace(hash, h);

    m_chunkLoaded.push_back(h);
}

void WorldSystem::buildMesh(Hash64 hash, Handle poolID)
{
    std::vector<MeshCache> meshes(TERRAIN_NUM_LOD_LEVEL);

    for (int i=0;i<TERRAIN_NUM_LOD_LEVEL;i++)
    {
        meshes[i].hash = hash << 8 | uint64_t(i);

        meshes[i].mesh = CreateMesh(hash, i, meshes[i].radius);
    }

    MainThreadDispatcher::Add(Priority::NORMAL, &WorldSystem::UploadMesh, this, poolID, meshes);
}

void WorldSystem::UploadMesh(Handle poolID, std::vector<MeshCache> meshes)
{
    auto chunk = m_pool.get(poolID);
    if (chunk == nullptr || chunk->state != ChunkState::LOADING)
        return;

    for (int i=0;i<TERRAIN_NUM_LOD_LEVEL;i++)
    {
        auto& cache = meshes[i];

        MeshHandle handle = AssetsManager::FindMesh(cache.hash);
        if (handle == HANDLE_INVALID)
        {
            handle = AssetsManager::CreateMesh(cache.hash);

            auto mesh = AssetsManager::GetMesh(handle);
            *mesh = cache.mesh;
        }

        chunk->mesh[i] = handle;
        chunk->radius[i] = cache.radius;
    }

    chunk->state = ChunkState::LOADED;
}

bool WorldSystem::has(Hash64 hash) const
{
    return m_chunkNameMap.find(hash) != m_chunkNameMap.end();
}

void WorldSystem::unloadChunk(Handle h)
{
    StreamChunk* chunk = m_pool.get(h);

    for (int i=0;i<TERRAIN_NUM_LOD_LEVEL;i++)
    {
        if (chunk->mesh[i] != HANDLE_INVALID)
        {
            auto mesh = AssetsManager::GetMesh(chunk->mesh[i]);
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

                AssetsManager::RemoveMesh(chunk->mesh[i]);
            }
        }
    }

    m_pool.destroy(h);
}

void WorldSystem::render(StreamChunk* chunk, const Vector3& camPos)
{
    if (chunk->state != ChunkState::LOADED)
        return;

    float worldX = (float)((int)handle_get_first(chunk->hash)) * TERRAIN_CHUNK_SIZE;
    float worldZ = (float)((int)handle_get_second(chunk->hash)) * TERRAIN_CHUNK_SIZE;

    Vector3 position = { worldX, 0.0f, worldZ };

    int lodLevel = 0;
    float distance = Math::distance(position, camPos);

    if (distance >= 150.0f + 5.0f) 
        lodLevel = 3;
    else if (distance >= 100.0f + 5.0f) 
        lodLevel = 2;
    else if (distance >= 50.0f + 5.0f)
        lodLevel = 1;
    else
        lodLevel = 0;

    float radius = chunk->radius[lodLevel];

    if (!Frustum::IsSphereInFrustum(position, radius))
        return;

    Vector3 p1{worldX,              0, worldZ};
    Vector3 p2{worldX + TERRAIN_CHUNK_SIZE, 0, worldZ};
    Vector3 p3{worldX + TERRAIN_CHUNK_SIZE, 0, worldZ + TERRAIN_CHUNK_SIZE};
    Vector3 p4{worldX,              0, worldZ + TERRAIN_CHUNK_SIZE};

    DebugDraw::DrawLine(p1, p2);
    DebugDraw::DrawLine(p2, p3);
    DebugDraw::DrawLine(p3, p4);
    DebugDraw::DrawLine(p4, p1);

    Renderer::Submit(chunk->material, chunk->mesh[lodLevel], 0, &chunk->transform);
    // Renderer::Submit(chunk->material, chunk->mesh, 0, &Matrix4x4::identityMatrix);
}

struct MyVertex3D
{
    Vector3 position;
    Vector3 normal;
    Vector2 texCoords;
};

#define STB_PERLIN_IMPLEMENTATION
#include "gr-base/core/stb/stb_perlin.h"

float fbm(float x, float y)
{
    float total = 0.0f;
    float frequency = 1.0f / 256.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < 8; i++)
    {
        total += stb_perlin_noise3(
            x * frequency,
            y * frequency,
            0,
            0,0,0
        ) * amplitude;

        maxValue += amplitude;

        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / maxValue;
}

float ridged(float x, float y)
{
    float n = fbm(x, y);

    n = fabs(n);
    n = 1.0f - n;

    return n * n;
}

float get_terrain_height(float x, float y)
{
    float continents = fbm(x * 0.15f, y * 0.15f);

    float hills = fbm(x, y);

    float mountains = ridged(x * 2.0f, y * 2.0f);

    float h = 0.0f;

    h += continents * 50.0f;
    h += hills * 15.0f;
    // h += mountains * 100.0f;

    if (continents > 0.2f)
    {
        float mask = (continents - 0.2f) / 0.8f;
        h += mountains * mask * 100.0f;
    }

    h = std::max(0.0f, h);
    return h;
}

Mesh WorldSystem::CreateMesh(Hash64 hash, int lodLevel, float& radius)
{
    float spacing = 1.0f;

    std::vector<MyVertex3D> vertices;
    std::vector<uint32_t> indices;

    int step = 1 << lodLevel;
    
    if (step > TERRAIN_CHUNK_SIZE)
        step = TERRAIN_CHUNK_SIZE;

    radius = 0.0f;

    Vector3 center = Vector3::zero;

    for (int x = 0; x <= TERRAIN_CHUNK_SIZE; x+=step)
    {
        for (int z = 0; z <= TERRAIN_CHUNK_SIZE; z+=step)
        {
            float localX = (float)x / (TERRAIN_CHUNK_SIZE - 1);
            float localZ = (float)z / (TERRAIN_CHUNK_SIZE - 1);

            int worldX = ((int)handle_get_first(hash)) * TERRAIN_CHUNK_SIZE + x;
            int worldZ = ((int)handle_get_second(hash)) * TERRAIN_CHUNK_SIZE + z;

            float heigth = get_terrain_height(worldX, worldZ);

            MyVertex3D vertex{};
            vertex.position = {
                localX,
                heigth,
                localZ
            };

            center += vertex.position;

            vertex.normal = { 0.0f, 0.0f, 0.0f };

            vertex.texCoords = {
                localX,
                localZ
            };

            vertices.push_back(vertex);
        }
    }

    center /= float(vertices.size());
    float maxRadius = 0.0f;
    for (const auto& vertex : vertices) {
        float distSq = Math::magnitude(vertex.position - center);

        maxRadius = std::max(maxRadius, distSq);

        // if (distSq > maxRadiusSq) {
            // maxRadiusSq = distSq * TERRAIN_CHUNK_SIZE;
        // }
    }

    radius = maxRadius * TERRAIN_CHUNK_SIZE;

    int reducedSize = (TERRAIN_CHUNK_SIZE / step) + 1;
    
    for (int x = 0; x < reducedSize - 1; x++)
    {
        for (int z = 0; z < reducedSize - 1; z++)
        {
            int topLeft     = x * (reducedSize) + z;
            int topRight    = topLeft + 1;
            int bottomLeft  = (x + 1) * (reducedSize) + z;
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

        v0.normal += kNormal;
        v1.normal += kNormal;
        v2.normal += kNormal;
    }

    for (auto& v : vertices)
        v.normal = Math::normalize(v.normal);

    std::vector<Vertex3D> openglvertices;
    for (auto& v : vertices)
        openglvertices.push_back({v.position, v.normal, v.texCoords});

    Mesh mesh;
    mesh.vertices = (Vertex3D*)malloc(openglvertices.size() * sizeof(Vertex3D));
    mesh.numVertices = openglvertices.size();

    memcpy(mesh.vertices, openglvertices.data(), openglvertices.size() * sizeof(Vertex3D));

    Submesh submesh;
    submesh.indices = (uint32_t*)malloc(indices.size() * sizeof(uint32_t));
    submesh.numIndices = indices.size();

    memcpy(submesh.indices, indices.data(), indices.size() * sizeof(uint32_t));

    std::vector<Submesh> submeshes;
    submeshes.push_back(submesh);

    mesh.submeshes = (Submesh*)malloc(submeshes.size() * sizeof(Submesh));
    mesh.numSubmeshes = submeshes.size();

    memcpy(mesh.submeshes, submeshes.data(), submeshes.size() * sizeof(Submesh));

    return mesh;
}


