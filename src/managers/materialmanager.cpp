#include "gr-base/managers/materialmanager.hpp"

#include "gr-base/core/geometry/material.hpp"

#include <cstdlib>

MaterialManager::MaterialManager()
{}

MaterialID MaterialManager::create()
{
    return { m_pool.emplace() };
}

Material* MaterialManager::get(MaterialID h) const
{
    return m_pool.try_get(h.value);
}

void MaterialManager::destroy(MaterialID h)
{
    m_pool.destroy(h.value);
}

size_t MaterialManager::GetCount() const
{
    return m_pool.count();
}

void MaterialManager::addInCache(AssetID assetID, MaterialID index)
{
    // uint32_t index = handle_get_first(handle);
    if (index.value >= materialIDCacheList.size())
        materialIDCacheList.resize(index.value + 1, INVALID_ASSET);

    materialIDCacheList[index.value] = assetID;
}

AssetID MaterialManager::findInCache(MaterialID index) const
{
    // uint32_t index = handle_get_first(handle);
    if (index.value >= materialIDCacheList.size())
        return INVALID_ASSET;

    return materialIDCacheList[index.value];
}



