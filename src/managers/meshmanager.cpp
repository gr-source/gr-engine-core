#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <string.h>

#include <cassert>
#include <cstdlib>

MeshManager::MeshManager()
{}

MeshID MeshManager::create()
{
    return m_pool.emplace();
}

Mesh *MeshManager::get(MeshID id) const
{
    return m_pool.try_get(id);
}

void MeshManager::remove(MeshID id)
{
    m_pool.destroy(id);
}

size_t MeshManager::GetCount() const
{
    return m_pool.count();
}

void MeshManager::addInCache(AssetID assetID, MeshID index)
{
    if (index >= meshIDCache.size())
        meshIDCache.resize(index + 1, INVALID_ASSET);

    meshIDCache[index] = assetID;
}

AssetID MeshManager::findInCache(MeshID index) const
{
    if (index >= meshIDCache.size())
        return INVALID_ASSET;

    return meshIDCache[index];
}




