#include "gr-base/core/assetPtr.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/texture.hpp>

// ===== Texture ===== //
template <>
gr::texture* AssetPtr<gr::texture, TextureID>::get()
{
    return AssetsManager::GetTexture(id);
}

template <>
const gr::texture* AssetPtr<gr::texture, TextureID>::get() const
{
    return AssetsManager::GetTexture(id);
}

// ===== Texture ===== //
template <>
Mesh* AssetPtr<Mesh, MeshID>::get()
{
    return AssetsManager::GetMesh(id);
}

template <>
const Mesh* AssetPtr<Mesh, MeshID>::get() const
{
    return AssetsManager::GetMesh(id);
}

// ===== Material ===== //
template <>
Material* AssetPtr<Material, MaterialID>::get()
{
    return AssetsManager::GetMaterial(id);
}

template <>
const Material* AssetPtr<Material, MaterialID>::get() const
{
    return AssetsManager::GetMaterial(id);
}


