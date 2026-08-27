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

    Mesh *get(MeshID id) const;

    void remove(MeshID id);

    size_t GetCount() const;

    void addInCache(AssetID assetID, MeshID index);

    AssetID findInCache(MeshID index) const;

private:
    std::vector<AssetID> meshIDCache;

    SparseSet2<Mesh> m_pool;
};



