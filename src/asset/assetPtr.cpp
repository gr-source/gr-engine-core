#include "gr-base/asset/assetPtr.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/gTexture.h>

// ===== Texture ===== //
template <>
gr::Texture* AssetPtr<gr::Texture>::get()
{
    return AssetsManager::GetTexture(handle);
}

template <>
gr::Texture* AssetPtr<gr::Texture>::operator->()
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


