#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/externals/handle.h"

#include <string.h>

#include <cassert>
#include <cstdlib>

MeshManager::MeshManager()
{}

MeshID MeshManager::create()
{
    return m_pool.emplace();
}

Mesh *MeshManager::get(MeshID h) const
{
    return m_pool.try_get(h);
}

void MeshManager::remove(MeshID h)
{
    m_pool.destroy((uint32_t)h);
}

size_t MeshManager::GetCount() const
{
    return m_pool.count();
}

void MeshManager::addInCache(AssetID assetID, MeshID index)
{
    // uint32_t index = handle_get_first(handle);
    if (index >= meshIDCache.size())
        meshIDCache.resize(index + 1, INVALID_ASSET);

    meshIDCache[index] = assetID;
}

AssetID MeshManager::findInCache(MeshID index) const
{
    // uint32_t index = handle_get_first(handle);
    if (index >= meshIDCache.size())
        return INVALID_ASSET;

    return meshIDCache[index];
}




