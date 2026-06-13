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
        float radius;
    };

    std::unordered_map<Hash64, Handle> m_chunkNameMap;

    std::vector<Handle> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    void create(Hash64 hash);

    void buildMesh(Hash64 hash, Handle poolID);

    Mesh CreateMesh(Hash64 hash, int lodLevel, float& radius);

    void UploadMesh(Handle poolID, std::vector<MeshCache> meshes);

    bool has(Hash64 hash) const;

    void unloadChunk(Handle h);

    void render(StreamChunk* chunk, const Vector3& camPos);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<WorldSystem> =
    TypeCategory::System;

