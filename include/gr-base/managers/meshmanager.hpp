#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include <unordered_map>

#define INVALID_MESHID UINT32_MAX

typedef uint32_t MeshID;

class MeshManager
{
public:
    MeshManager();

    MeshHandle create();

    Mesh *get(MeshHandle h) const;

    void remove(MeshHandle h);

    size_t GetCount() const;

    void addInCache(AssetID assetID, MeshHandle handle);

    AssetID findInCache(MeshHandle handle);

private:
    struct Slot
    {
        MeshID next;
        uint32_t version;
    };

    Mesh* m_pool;

    size_t m_capacity;

    size_t m_count;

    Slot *m_slots;
    MeshID m_freeID;

    std::vector<AssetID> meshIDCache;

    void resize();

    void addFreeID(size_t oldCapacity);
};



