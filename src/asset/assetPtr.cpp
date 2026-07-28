#include "gr-base/asset/assetPtr.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/texture.h>

// ===== Texture ===== //
template <>
gr::texture* AssetPtr<gr::texture>::get()
{
    return AssetsManager::GetTexture(handle);
}

template <>
gr::texture* AssetPtr<gr::texture>::operator->()
{
    return get();
}

// ===== Material ===== //
template <>
Material* AssetPtr<Material>::get()
{
    return AssetsManager::GetMaterial(handle);
}

template <>
Material* AssetPtr<Material>::operator->()
{
    return get();
}


