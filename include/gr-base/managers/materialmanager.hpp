#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/externals/hash.hpp"

#include "gr-base/managers/default.hpp"

#include <unordered_map>
#include <vector>

class MaterialManager
{
public:
    MaterialManager();

    MaterialHandle create(Hash64 hash);

    Material *get(MaterialHandle h);

    void destroy(MaterialHandle h);

    MaterialHandle find(Hash64 hash) const;

    size_t GetCount() const;

private:
    struct Slot
    {
        uint32_t next;
        uint32_t version;
    };

    std::unordered_map<Hash64, MaterialHandle> m_materialMap;

    std::vector<Material> m_materialPool;

    size_t m_capacity;

    size_t m_count;

    Slot *m_slots;
    uint32_t m_freeID;

    void resize();

    void addFreeID(size_t oldCapacity);
};



