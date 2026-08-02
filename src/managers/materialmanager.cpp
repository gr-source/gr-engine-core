#include "gr-base/managers/materialmanager.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/externals/handle.h"

#include <cstdlib>

MaterialManager::MaterialManager()
{}

MaterialHandle MaterialManager::create()
{
    return m_pool.create();
}

Material *MaterialManager::get(MaterialHandle h) const
{
    return m_pool.get(h);
}

void MaterialManager::destroy(MaterialHandle h)
{
    m_pool.destroy(h);
}

size_t MaterialManager::GetCount() const
{
    return m_pool.count();
}

void MaterialManager::addInCache(AssetID assetID, MaterialHandle handle)
{
    uint32_t index = handle_get_first(handle);
    if (index >= materialIDCacheList.size())
        materialIDCacheList.resize(index + 1, INVALID_ASSET);

    materialIDCacheList[index] = assetID;
}

AssetID MaterialManager::findInCache(MaterialHandle handle) const
{
    uint32_t index = handle_get_first(handle);
    if (index >= materialIDCacheList.size())
        return INVALID_ASSET;

    return materialIDCacheList[index];
}



