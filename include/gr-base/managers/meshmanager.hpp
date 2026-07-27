#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include <unordered_map>

class MeshManager
{
public:
    MeshManager();

    MeshHandle create();

    Mesh *get(MeshHandle h) const;

    void remove(MeshHandle h);

    size_t GetCount() const;

private:
    struct Slot
    {
        uint32_t next;
        uint32_t version;
    };

    Mesh* m_pool;

    size_t m_capacity;

    size_t m_count;

    Slot *m_slots;
    uint32_t m_freeID;

    void resize();

    void addFreeID(size_t oldCapacity);
};



