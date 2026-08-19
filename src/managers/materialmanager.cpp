#include "gr-base/managers/materialmanager.hpp"

#include "gr-base/core/geometry/material.hpp"

#include <cstdlib>

MaterialManager::MaterialManager()
{}

MaterialID MaterialManager::create()
{
    return m_pool.emplace();
}

Material& MaterialManager::get(MaterialID h) const
{
    return m_pool.get(h);
}

void MaterialManager::destroy(MaterialID h)
{
    m_pool.destroy(h);
}

size_t MaterialManager::GetCount() const
{
    return m_pool.count();
}

void MaterialManager::addInCache(AssetID assetID, MaterialID index)
{
    // uint32_t index = handle_get_first(handle);
    if (index >= materialIDCacheList.size())
        materialIDCacheList.resize(index + 1, INVALID_ASSET);

    materialIDCacheList[index] = assetID;
}

AssetID MaterialManager::findInCache(MaterialID index) const
{
    // uint32_t index = handle_get_first(handle);
    if (index >= materialIDCacheList.size())
        return INVALID_ASSET;

    return materialIDCacheList[index];
}



