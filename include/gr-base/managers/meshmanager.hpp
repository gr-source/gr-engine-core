#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include "gr-base/pool/sparseSet.hpp"

class MeshManager
{
public:
    MeshManager();

    MeshID create();

    Mesh *get(MeshID h) const;

    void remove(MeshID h);

    size_t GetCount() const;

    void addInCache(AssetID assetID, MeshID handle);

    AssetID findInCache(MeshID handle) const;

private:
    std::vector<AssetID> meshIDCache;

    SparseSet2<Mesh> m_pool;
};



