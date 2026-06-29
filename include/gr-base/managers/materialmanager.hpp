#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/externals/hash.hpp"

#include <unordered_map>
#include <vector>

class MaterialManager
{
public:
    MaterialManager();

    MaterialHandle create(AssetHash hash);

    MaterialHandle find(AssetHash hash) const;

    Material *get(MaterialHandle h);

    void destroy(MaterialHandle h);

    size_t GetCount() const;

private:
    struct Slot
    {
        uint32_t next;
        uint32_t version;
        AssetHash hash;
    };

    std::unordered_map<AssetHash, MaterialHandle> m_materialMap;

    std::vector<Material> m_materialPool;

    size_t m_capacity;

    size_t m_count;

    Slot *m_slots;
    uint32_t m_freeID;

    void resize();

    void addFreeID(size_t oldCapacity);
};



