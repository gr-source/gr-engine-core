#pragma once

#include "gr-base/core/geometry/mesh.hpp"
#include "gr-base/core/system.hpp"

#include "gr-base/managers/default.hpp"
#include "gr-base/system/world/cell.hpp"

#include "gr-base/pool/poolObject.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/type_traits.hpp"

#include <unordered_map>
#include <vector>
#include <string>


class WorldSystem : public ISystem
{
public:
    WorldSystem();
    ~WorldSystem() override;

    void Update(Scene* scene) override;

    int CunkSize;

    int RenderDistance;

private:
    struct MeshCache
    {
        Mesh mesh;
        uint64_t hash;
        BoundingSphere bounds;
    };

    std::unordered_map<ChunkCoord, Handle> m_chunkNameMap;

    std::vector<Handle> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    void create(ChunkCoord coord);

    void buildMesh(ChunkCoord coord, Handle poolID);

    Mesh CreateMesh(ChunkCoord coord, int lodLevel);

    void UploadMesh(Handle poolID, const Matrix4x4 transform, std::vector<MeshCache> meshes);

    bool has(ChunkCoord coord) const;

    void unloadChunk(Handle h);

    void render(StreamChunk* chunk, const Vector3& camPos);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<WorldSystem> =
    TypeCategory::System;

