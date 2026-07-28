#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include <unordered_map>
#include <vector>

class MaterialManager
{
public:
    MaterialManager();

    MaterialHandle create();

    Material *get(MaterialHandle h);

    void destroy(MaterialHandle h);

    size_t GetCount() const;

    void addInCache(AssetID assetID, MaterialHandle handle);

    AssetID findInCache(MaterialHandle handle);

private:
    struct Slot
    {
        uint32_t next;
        uint32_t version;
    };

    std::vector<Material> m_materialPool;

    size_t m_capacity;

    size_t m_count;

    Slot *m_slots;
    uint32_t m_freeID;

    std::vector<AssetID> meshIDCache;

    void resize();

    void addFreeID(size_t oldCapacity);
};



