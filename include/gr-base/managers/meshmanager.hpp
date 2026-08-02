#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include "gr-base/pool/objectPool.hpp"

class MeshManager
{
public:
    MeshManager();

    MeshHandle create();

    Mesh *get(MeshHandle h) const;

    void remove(MeshHandle h);

    size_t GetCount() const;

    void addInCache(AssetID assetID, MeshHandle handle);

    AssetID findInCache(MeshHandle handle) const;

private:
    std::vector<AssetID> meshIDCache;

    ObjectPool<Mesh> m_pool;
};



