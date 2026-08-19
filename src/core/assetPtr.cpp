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
const gr::texture* AssetPtr<gr::texture>::get() const
{
    return AssetsManager::GetTexture(handle);
}

// ===== Texture ===== //
template <>
Mesh* AssetPtr<Mesh>::get()
{
    return AssetsManager::GetMesh(handle);
}

template <>
const Mesh* AssetPtr<Mesh>::get() const
{
    return AssetsManager::GetMesh(handle);
}

// ===== Material ===== //
template <>
Material* AssetPtr<Material>::get()
{
    return &AssetsManager::GetMaterial(handle);
}

template <>
const Material* AssetPtr<Material>::get() const
{
    return &AssetsManager::GetMaterial(handle);
}


