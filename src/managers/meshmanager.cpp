#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/externals/handle.h"

#include <cassert>
#include <cstdlib>

MeshManager::MeshManager()
{}

MeshHandle MeshManager::create()
{
    return m_pool.create();
}

Mesh *MeshManager::get(MeshHandle h) const
{
    return m_pool.get(h);
}

void MeshManager::remove(MeshHandle h)
{
    m_pool.destroy(h);
}

size_t MeshManager::GetCount() const
{
    return m_pool.count();
}

void MeshManager::addInCache(AssetID assetID, MeshHandle handle)
{
    uint32_t index = handle_get_first(handle);
    if (index >= meshIDCache.size())
        meshIDCache.resize(index + 1, INVALID_ASSET);

    meshIDCache[index] = assetID;
}

AssetID MeshManager::findInCache(MeshHandle handle) const
{
    uint32_t index = handle_get_first(handle);
    if (index >= meshIDCache.size())
        return INVALID_ASSET;

    return meshIDCache[index];
}




