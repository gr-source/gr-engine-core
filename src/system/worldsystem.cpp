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

    int worldX = ((int)floor(transform->position.x)) >> TERRAIN_CHUNK_BITS;
    int worldZ = ((int)floor(transform->position.z)) >> TERRAIN_CHUNK_BITS;

    for (int x=-TERRAIN_LOAD_DISTANCE;x<=TERRAIN_LOAD_DISTANCE;x++)
    {
        for (int z=-TERRAIN_LOAD_DISTANCE;z<=TERRAIN_LOAD_DISTANCE;z++)
        {
            int chunkIdX = worldX + x;
            int chunkIdZ = worldZ + z;

            // bitpack
            ChunkCoord coord = handle_create(chunkIdX, chunkIdZ);

            if (!has(coord))
                create(coord);
        }
    }

    for (auto it = m_chunkLoaded.begin(); it != m_chunkLoaded.end(); )
    {
        auto chunk = m_pool.get(*it);

        int dx = ((int)handle_get_first(chunk->coord)) - worldX;
        int dz = ((int)handle_get_second(chunk->coord)) - worldZ;

        int distance = std::max(std::abs(dx), std::abs(dz));

        if (distance > TERRAIN_UNLOAD_DISTANCE)
        {
            if (chunk->state != ChunkState::LOADING)
            {
                m_chunkNameMap.erase(chunk->coord);

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

void WorldSystem::create(ChunkCoord coord)
{
    Handle h = m_pool.create();

    StreamChunk* chunk = m_pool.get(h);
    chunk->coord = coord;

    chunk->state = ChunkState::LOADING;
    chunk->transform = Matrix4x4::identityMatrix;

    Threadpool::AddTask(&WorldSystem::buildMesh, this, coord, h);

    chunk->material = AssetsManager::FindMaterial(engine::hash::fnv1_str("material/Tree.Birch.Leaf.Summer.Mat"));
    if (chunk->material == HANDLE_INVALID)
        chunk->material = AssetsManager::LoadMaterial(engine::hash::fnv1_str("material/Tree.Birch.Leaf.Summer.Mat"));

    m_chunkNameMap.emplace(coord, h);

    m_chunkLoaded.push_back(h);
}

static BoundingSphere CalculateBoundsSphere(Mesh* mesh)
{
    BoundingSphere sphere;

    if (!mesh->numVertices)
    {
        sphere.center = Vector3::zero;
        sphere.radius = 0.0f;
        return sphere;
    }

    // 1. Primeiro calcula AABB (reaproveitando sua lógica)
    Vector3 min = mesh->vertices[0].position;
    Vector3 max = mesh->vertices[0].position;

    for (size_t i = 0; i < mesh->numVertices; i++)
    {
        const auto& p = mesh->vertices[i].position;

        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        min.z = std::min(min.z, p.z);

        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        max.z = std::max(max.z, p.z);
    }

    // 2. Centro da esfera = centro do AABB
    sphere.center = (min + max) * 0.5f;

    // 3. Raio = maior distância até qualquer vértice
    float radiusSq = 0.0f;

    for (size_t i = 0; i < mesh->numVertices; i++)
    {
        const auto& p = mesh->vertices[i].position;
        Vector3 d = p - sphere.center;

        radiusSq = std::max(radiusSq, Math::dot(d, d));
    }

    sphere.radius = std::sqrt(radiusSq);

    return sphere;
}

void WorldSystem::buildMesh(ChunkCoord coord, Handle poolID)
{
    std::vector<MeshCache> meshes(TERRAIN_NUM_LOD_LEVEL);

    int worldX = ((int)handle_get_first(coord)) << TERRAIN_CHUNK_BITS;
    int worldZ = ((int)handle_get_second(coord)) << TERRAIN_CHUNK_BITS;

    Vector3 position =
    {
        (float)worldX,
        0.0f,
        (float)worldZ
    };

    Matrix4x4 model = Math::CreateTRS(
        position,
        Quaternion::identity,
        Vector3{ float(TERRAIN_CHUNK_SIZE), 5.0f, float(TERRAIN_CHUNK_SIZE) }
    );

    for (int i=0;i<TERRAIN_NUM_LOD_LEVEL;i++)
    {
        auto& cache = meshes[i];
        // Calcula uma hash para a mesh baseado no bitpack da ChunkCoord
        cache.hash = coord << 8 | uint64_t(i);

        Mesh mesh = CreateMesh(coord, i);

        BoundingSphere bounds = CalculateBoundsSphere(&mesh);
        bounds.center = model * bounds.center;
        bounds.radius = bounds.radius * (float)TERRAIN_CHUNK_SIZE;

        cache.mesh = mesh;
        cache.bounds = bounds;
    }

    MainThreadDispatcher::Add(Priority::NORMAL, &WorldSystem::UploadMesh, this, poolID, model, meshes);
}

void WorldSystem::UploadMesh(Handle poolID, const Matrix4x4 transform, std::vector<MeshCache> meshes)
{
    auto chunk = m_pool.get(poolID);
    if (chunk == nullptr || chunk->state != ChunkState::LOADING)
        return;
    chunk->transform = transform;

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

        chunk->meshLOD[i] = {
            .mesh = handle,
            .bounds = cache.bounds
        };
    }

    chunk->state = ChunkState::LOADED;
}

bool WorldSystem::has(ChunkCoord coord) const
{
    return m_chunkNameMap.find(coord) != m_chunkNameMap.end();
}

void WorldSystem::unloadChunk(Handle h)
{
    StreamChunk* chunk = m_pool.get(h);

    for (int i=0;i<TERRAIN_NUM_LOD_LEVEL;i++)
    {
        auto& meshLOD = chunk->meshLOD[i];

        if (meshLOD.mesh != HANDLE_INVALID)
        {
            auto mesh = AssetsManager::GetMesh(meshLOD.mesh);
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

                AssetsManager::RemoveMesh(meshLOD.mesh);
            }
        }
    }

    m_pool.destroy(h);
}

static void DrawDebugSphere(const Vector3& center, float radius, int segments = 16)
{
    const float step = 2.0f * 3.14159265f / segments;

    // Círculo XZ (horizontal)
    for (int i = 0; i < segments; i++)
    {
        float a0 = i * step;
        float a1 = (i + 1) * step;

        Vector3 p0 = {
            center.x + cosf(a0) * radius,
            center.y,
            center.z + sinf(a0) * radius
        };

        Vector3 p1 = {
            center.x + cosf(a1) * radius,
            center.y,
            center.z + sinf(a1) * radius
        };

        DebugDraw::DrawLine(p0, p1);
    }

    // Círculo XY
    for (int i = 0; i < segments; i++)
    {
        float a0 = i * step;
        float a1 = (i + 1) * step;

        Vector3 p0 = {
            center.x + cosf(a0) * radius,
            center.y + sinf(a0) * radius,
            center.z
        };

        Vector3 p1 = {
            center.x + cosf(a1) * radius,
            center.y + sinf(a1) * radius,
            center.z
        };

        DebugDraw::DrawLine(p0, p1);
    }

    // Círculo YZ
    for (int i = 0; i < segments; i++)
    {
        float a0 = i * step;
        float a1 = (i + 1) * step;

        Vector3 p0 = {
            center.x,
            center.y + cosf(a0) * radius,
            center.z + sinf(a0) * radius
        };

        Vector3 p1 = {
            center.x,
            center.y + cosf(a1) * radius,
            center.z + sinf(a1) * radius
        };

        DebugDraw::DrawLine(p0, p1);
    }
}

void WorldSystem::render(StreamChunk* chunk, const Vector3& camPos)
{
    if (chunk->state != ChunkState::LOADED)
        return;

    Vector3 position =
    {
        (float)(((int)handle_get_first(chunk->coord)) << TERRAIN_CHUNK_BITS),
        0.0f,
        (float)(((int)handle_get_second(chunk->coord)) << TERRAIN_CHUNK_BITS)
    };

    int lodLevel = 0;
    float distance = Math::distance(position, camPos);

    if (distance >= 300.0f + 5.0f)
        lodLevel = 3;
    else if (distance >= 200.0f + 5.0f) 
        lodLevel = 2;
    else if (distance >= 100.0f + 5.0f)
        lodLevel = 1;
    else
        lodLevel = 0;

    auto& meshLOD = chunk->meshLOD[lodLevel];

    auto& sphere = meshLOD.bounds;
    if (!Frustum::IsSphereInFrustum(sphere.center, sphere.radius))
        return;

    Vector3 p1{ position.x,                      0.0f, position.z };
    Vector3 p2{ position.x + TERRAIN_CHUNK_SIZE, 0.0f, position.z };
    Vector3 p3{ position.x + TERRAIN_CHUNK_SIZE, 0.0f, position.z + TERRAIN_CHUNK_SIZE};
    Vector3 p4{ position.x,                      0.0f, position.z + TERRAIN_CHUNK_SIZE};

    DebugDraw::DrawLine(p1, p2);
    DebugDraw::DrawLine(p2, p3);
    DebugDraw::DrawLine(p3, p4);
    DebugDraw::DrawLine(p4, p1);

    Renderer::Submit(chunk->material, meshLOD.mesh, 0, &chunk->transform);
    // Renderer::Submit(chunk->material, chunk->mesh, 0, &Matrix4x4::identityMatrix);
}

#define STB_PERLIN_IMPLEMENTATION
#include "gr-base/core/stb/stb_perlin.h"

float FBM(float x, float z, int octaves, float gain, float lacunarity)
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float max_value = 0.0f;

    for (int i = 0; i < octaves; i++)
    {
        float sampleX = x * frequency;
        float sampleZ = z * frequency;

        float height = stb_perlin_noise3(sampleX, sampleZ, 0, 0, 0, 0) * 2.0f - 1.0f;

        total += height * amplitude;
        
        max_value += amplitude;
        amplitude *= gain;
        frequency *= lacunarity;
    }

    return total / max_value;
}

Mesh WorldSystem::CreateMesh(ChunkCoord coord, int lodLevel)
{
    std::vector<Vertex3D> vertices;
    std::vector<uint32_t> indices;

    // =========================
    // LOD → vertex count (CORRETO)
    // =========================
    int vertexCount = (TERRAIN_CHUNK_SIZE >> lodLevel) + 1;
    vertexCount = std::max(2, vertexCount);

    int step = TERRAIN_CHUNK_SIZE / (vertexCount - 1);

    // Unpack
    int chunkX = (int)handle_get_first(coord);
    int chunkZ = (int)handle_get_second(coord);

    float frequency = 0.01f;

    // =========================
    // VERTEX GENERATION
    // =========================
    for (int ix = 0; ix < vertexCount; ix++)
    {
        for (int iz = 0; iz < vertexCount; iz++)
        {
            int x = ix * step;
            int z = iz * step;

            float localX = (float)x / TERRAIN_CHUNK_SIZE;
            float localZ = (float)z / TERRAIN_CHUNK_SIZE;

            int worldX = (chunkX << TERRAIN_CHUNK_BITS) + x;
            int worldZ = (chunkZ << TERRAIN_CHUNK_BITS) + z;


            const float continentFreq = 0.0015f;
            const float hillFreq      = 0.01f;
            const float detailFreq    = 0.08f;

            float continent = stb_perlin_fbm_noise3(
                worldX * continentFreq,
                worldZ * continentFreq,
                0.0f,
                2.0f,
                0.5f,
            5);

        continent = continent * 0.5f + 0.5f; // [-1,1] -> [0,1]

        float hills = stb_perlin_ridge_noise3(
            worldX * hillFreq,
            worldZ * hillFreq,
            0.0f,
            2.0f,
            0.5f,
            1.0f,
            4);

        float details = stb_perlin_noise3(
            worldX * detailFreq,
            worldZ * detailFreq,
            0.0f,
            0,
            0,
            0);

            float height = 0.0f;

            // oceanos e continentes
            height += continent * 120.0f;

            // montanhas apenas sobre terra
            height += hills * continent * 60.0f;

            // pequenos detalhes
            height += details * 4.0f;

            // float height = FBM(worldX, worldZ, 4, 0.5f, 2.0f);

            Vertex3D vertex{};
            vertex.position = {
                localX,
                height,
                localZ
            };

            vertex.normal = { 0.0f, 0.0f, 0.0f };

            vertex.texCoords = {
                localX,
                localZ
            };

            vertices.push_back(vertex);
        }
    }

    // =========================
    // INDEX GENERATION
    // =========================
    for (int ix = 0; ix < vertexCount - 1; ix++)
    {
        for (int iz = 0; iz < vertexCount - 1; iz++)
        {
            int topLeft     = ix * vertexCount + iz;
            int topRight    = topLeft + 1;
            int bottomLeft  = (ix + 1) * vertexCount + iz;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);

            indices.push_back(topRight);
            indices.push_back(bottomRight);
            indices.push_back(bottomLeft);
        }
    }

    // =========================
    // NORMALS
    // =========================
    for (uint32_t i = 0; i < indices.size(); i += 3)
    {
        auto& v0 = vertices[indices[i + 0]];
        auto& v1 = vertices[indices[i + 1]];
        auto& v2 = vertices[indices[i + 2]];

        Vector3 normal = Math::normalize(
            Math::cross(v1.position - v0.position,
                        v2.position - v0.position)
        );

        v0.normal += normal;
        v1.normal += normal;
        v2.normal += normal;
    }

    for (auto& v : vertices)
        v.normal = Math::normalize(v.normal);

    // =========================
    // OUTPUT MESH
    // =========================
    Mesh mesh;

    mesh.vertices = (Vertex3D*)malloc(vertices.size() * sizeof(Vertex3D));
    mesh.numVertices = vertices.size();
    memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(Vertex3D));

    Submesh submesh;
    submesh.indices = (uint32_t*)malloc(indices.size() * sizeof(uint32_t));
    submesh.numIndices = indices.size();
    memcpy(submesh.indices, indices.data(), indices.size() * sizeof(uint32_t));

    mesh.submeshes = (Submesh*)malloc(sizeof(Submesh));
    mesh.numSubmeshes = 1;
    mesh.submeshes[0] = submesh;

    return mesh;
}


