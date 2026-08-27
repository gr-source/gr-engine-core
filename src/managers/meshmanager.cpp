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
    return {m_pool.emplace()};
}

Mesh *MeshManager::get(MeshID h) const
{
    return m_pool.try_get(h.value);
}

void MeshManager::remove(MeshID h)
{
    m_pool.destroy(h.value);
}

size_t MeshManager::GetCount() const
{
    return m_pool.count();
}

void MeshManager::addInCache(AssetID assetID, MeshID index)
{
    // uint32_t index = handle_get_first(handle);
    if (index.value >= meshIDCache.size())
        meshIDCache.resize(index.value + 1, INVALID_ASSET);

    meshIDCache[index.value] = assetID;
}

AssetID MeshManager::findInCache(MeshID index) const
{
    // uint32_t index = handle_get_first(handle);
    if (index.value >= meshIDCache.size())
        return INVALID_ASSET;

    return meshIDCache[index.value];
}




