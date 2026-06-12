#pragma once

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
    std::unordered_map<Hash64, Handle> m_chunkNameMap;

    std::vector<Handle> m_chunkLoaded;

    ObjectPool<StreamChunk> m_pool;

    MeshHandle CreateMesh(const ChunkCoord& coord);

    void create(const ChunkCoord& coord);

    bool has(Hash64 hash) const;

    void unloadChunk(Handle h);

    void render(StreamChunk* chunk);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<WorldSystem> =
    TypeCategory::System;

