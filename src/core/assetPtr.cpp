#include "gr-base/core/assetPtr.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/texture.hpp>

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

// ===== Texture ===== //
template <>
Mesh* AssetPtr<Mesh>::get()
{
    return AssetsManager::GetMesh(handle);
}

template <>
Mesh* AssetPtr<Mesh>::operator->()
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


