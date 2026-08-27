#include "gr-base/managers/materialmanager.hpp"

#include "gr-base/core/geometry/material.hpp"

#include <cstdlib>

MaterialManager::MaterialManager()
{}

MaterialID MaterialManager::create()
{
    return m_pool.emplace();
}

Material* MaterialManager::get(MaterialID id) const
{
    return m_pool.try_get(id);
}

void MaterialManager::destroy(MaterialID id)
{
    m_pool.destroy(id);
}

size_t MaterialManager::GetCount() const
{
    return m_pool.count();
}

void MaterialManager::addInCache(AssetID assetID, MaterialID index)
{
    if (index >= materialIDCacheList.size())
        materialIDCacheList.resize(index + 1, INVALID_ASSET);

    materialIDCacheList[index] = assetID;
}

AssetID MaterialManager::findInCache(MaterialID index) const
{
    if (index >= materialIDCacheList.size())
        return INVALID_ASSET;

    return materialIDCacheList[index];
}



