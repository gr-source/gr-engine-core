#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include "gr-base/pool/sparseSet.hpp"

#include <vector>

class MaterialManager
{
public:
    MaterialManager();

    MaterialID create();

    Material* get(MaterialID id) const;

    void destroy(MaterialID id);

    size_t GetCount() const;

    void addInCache(AssetID assetID, MaterialID index);

    AssetID findInCache(MaterialID index) const;

private:
    std::vector<AssetID> materialIDCacheList;

    SparseSet2<Material> m_pool;
};



