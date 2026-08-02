#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"
#include "gr-base/pool/objectPool.hpp"

#include <vector>

class MaterialManager
{
public:
    MaterialManager();

    MaterialHandle create();

    Material *get(MaterialHandle h) const;

    void destroy(MaterialHandle h);

    size_t GetCount() const;

    void addInCache(AssetID assetID, MaterialHandle handle);

    AssetID findInCache(MaterialHandle handle) const;

private:
    std::vector<AssetID> materialIDCacheList;

    ObjectPool<Material> m_pool;
};



