#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"
#include "gr-base/externals/hash.hpp"

#include "gr-base/managers/default.hpp"

#include <unordered_map>

class MeshManager
{
public:
    MeshManager();

    MeshHandle create(Hash64 hash);

    Mesh *get(MeshHandle h) const;

    void destroy(MeshHandle h);

    MeshHandle find(Hash64 hash) const;

    size_t GetCount() const;

private:
    struct Slot
    {
        uint32_t next;
        uint32_t version;
    };

    std::unordered_map<Hash64, MeshHandle> m_meshMap;

    Mesh* m_meshPool;

    size_t m_capacity;

    size_t m_count;

    Slot *m_slots;
    uint32_t m_freeID;

    void resize();

    void addFreeID(size_t oldCapacity);
};



