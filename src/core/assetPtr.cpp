#include "gr-base/core/assetPtr.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/texture.hpp>

// ===== Texture ===== //
template <>
gr::texture* AssetPtr<gr::texture>::get()
{
    return AssetsManager::GetTexture(id);
}

template <>
const gr::texture* AssetPtr<gr::texture>::get() const
{
    return AssetsManager::GetTexture(id);
}

// ===== Texture ===== //
template <>
Mesh* AssetPtr<Mesh>::get()
{
    return AssetsManager::GetMesh(id);
}

template <>
const Mesh* AssetPtr<Mesh>::get() const
{
    return AssetsManager::GetMesh(id);
}

// ===== Material ===== //
template <>
Material* AssetPtr<Material>::get()
{
    return &AssetsManager::GetMaterial(id);
}

template <>
const Material* AssetPtr<Material>::get() const
{
    return &AssetsManager::GetMaterial(id);
}


