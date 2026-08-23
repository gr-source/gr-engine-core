#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/taskmanager.hpp"

#include "gr-base/asset/materialasset.hpp"
#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/assetResolver.hpp"
#include "gr-base/core/assetDB.hpp"
#include "gr-base/core/logger.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/file/fileStream.hpp"
#include "gr-base/file/file.hpp"

#include "gr-base/externals/handle.h"
#include "gr-base/externals/hash.hpp"

#include "gr-base/managers/shaderprefabmanager.hpp"
#include "gr-base/managers/materialmanager.hpp"
#include "gr-base/managers/textureManager.hpp"
#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/task/modelloadtask.hpp"

#include <stdexcept>

#include <string.h>

AssetsManager* AssetsManager::s_AssetsManager = nullptr;

void AssetsManager::Initialize()
{
    if (s_AssetsManager == nullptr)
        s_AssetsManager = new AssetsManager();
}

void AssetsManager::Shutdown()
{
    if (s_AssetsManager != nullptr)
        delete s_AssetsManager;

    s_AssetsManager = nullptr;
}

AssetPtr<gr::texture> AssetsManager::LoadTexture(const std::string& uri)
{
    AssetID assetID = AssetDB::FindByHash(engine::hash::fnv1_str(uri));
    if (assetID != INVALID_ASSET)
        return LoadTexture(assetID);
    throw std::runtime_error("asset '" + uri + "' not found");
}

#include <gr-render/texture.hpp>
AssetPtr<gr::texture> AssetsManager::LoadTexture(AssetID id)
{
    TextureID h = s_AssetsManager->findHandleInCache(id);
    if (h != INVALID_ID32)
        return h;

    auto info = AssetDB::GetAsset(id);
    if (info == nullptr)
        throw std::runtime_error("asset not found");

    auto path = AssetResolver::Resolve(info->uri) + HEADER_TEXTURE_EXTENSION;
    if (!gfile::exists(path))
        throw std::runtime_error("file not found");

    TextureAsset asset;
    if (!ImportAssetFromFile(path, &asset))
        throw std::runtime_error("invalid binary");

    TextureDesc desc;

    desc.width = asset.width;
    desc.height = asset.height;

    desc.filter = TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
    desc.wrap = TEXTURE_WRAP_MIRROR;
    desc.type = TEXTURE_TYPE_2D;

    switch (asset.channels)
    {
        case 1:
            desc.format = TEXTURE_FORMAT_RED;
            break;

        case 2:
            desc.format = asset.hdr ? TEXTURE_FORMAT_RG16F : TEXTURE_FORMAT_RG;
            break;

        case 3:
            desc.format = asset.hdr ? TEXTURE_FORMAT_RGB16F : TEXTURE_FORMAT_RGB;
            break;

        case 4:
            desc.format = asset.hdr ? TEXTURE_FORMAT_RGBA16F : TEXTURE_FORMAT_RGBA;
            break;
        default:
            throw std::runtime_error("Unsupported channel count: " + std::to_string(asset.channels));
    }

    desc.pixels = asset.raw.data();

    AssetPtr<gr::texture> texture =
        CreateTexture(desc);

    s_AssetsManager->addInCache(id, texture.handle);

    return texture;
}

AssetPtr<gr::texture> AssetsManager::CreateTexture(const TextureDesc& desc)
{
    return s_AssetsManager->mTextureManager->create(desc);
}

void AssetsManager::DestroyTexture(AssetPtr<gr::texture> ptr)
{
    s_AssetsManager->mTextureManager->remove(ptr.handle);
}

gr::texture* AssetsManager::GetTexture(TextureID h)
{
    return s_AssetsManager->mTextureManager->get(h);
}

// ********** Mesh ********** //
/*
MeshID AssetsManager::LoadMesh(const std::string& name)
{
    return LoadMesh(engine::hash::fnv1_str(name.c_str()));
}
*/

#include "gr-base/asset/meshasset.hpp"

AssetPtr<Mesh> AssetsManager::LoadMesh(AssetID id)
{
    MeshID h = s_AssetsManager->findHandleInCache(id);
    if (h != INVALID_ID32)
        return h;

    auto info = AssetDB::GetAsset(id);
    if (info == nullptr)
        throw std::runtime_error("asset not found");

    auto path = AssetResolver::Resolve(info->uri) + HEADER_MESH_EXTENSION;
    if (!gfile::exists(path))
        throw std::runtime_error("file not found");

    MeshAsset asset;
    if (!ImportAssetFromFile(path, &asset))
        throw std::runtime_error("invalid binary");

    auto mesh = CreateMesh();

    auto& submeshes = asset.submeshes;
    auto& vertices = asset.vertices;
    auto& indices = asset.indices;

    {
        mesh->vertices = (Vertex3D*)malloc(vertices.size() * sizeof(Vertex3D));
        mesh->numVertices = static_cast<uint32_t>(vertices.size());

        memcpy(mesh->vertices, vertices.data(), vertices.size() * sizeof(Vertex3D));
    }

    {
        mesh->indices = (uint32_t*)malloc(indices.size() * sizeof(uint32_t));
        mesh->numIndices = static_cast<uint32_t>(indices.size());

        memcpy(mesh->indices, indices.data(), indices.size() * sizeof(uint32_t));
    }

    mesh->submeshes = (Submesh*)malloc(submeshes.size() * sizeof(Submesh));
    mesh->numSubmeshes = static_cast<uint32_t>(submeshes.size());

    for (size_t i=0;i<mesh->numSubmeshes;i++)
    {
        auto& assetSubmesh = submeshes[i];

        auto& submesh = mesh->submeshes[i];
        submesh.indexOffset = assetSubmesh.indexOffset;
        submesh.indexCount = assetSubmesh.indexCount;
    }

    s_AssetsManager->addInCache(id, mesh.handle);
    s_AssetsManager->mMeshManager->addInCache(id, mesh.handle);

    return mesh;
}

size_t AssetsManager::GetMeshCount()
{
    return s_AssetsManager->mMeshManager->GetCount();
}

AssetPtr<Mesh> AssetsManager::CreateMesh()
{
    return s_AssetsManager->mMeshManager->create();
}

void AssetsManager::RemoveMesh(AssetPtr<Mesh> ptr)
{
    s_AssetsManager->mMeshManager->remove(ptr.handle);
}

Mesh *AssetsManager::GetMesh(MeshID h)
{
    return s_AssetsManager->mMeshManager->get(h);
}

AssetPtr<Material> AssetsManager::LoadMaterial(AssetID id)
{
    MaterialID h = s_AssetsManager->findHandleInCache(id);
    if (h != INVALID_ID32)
        return h;

    auto info = AssetDB::GetAsset(id);
    if (info == nullptr)
        throw std::runtime_error("asset not found");

    auto path = AssetResolver::Resolve(info->uri) + HEADER_MATERIAL_EXTENSION;
    if (!gfile::exists(path))
        throw std::runtime_error("file not found");

    MaterialAsset asset;
    if (!ImportAssetFromFile(path, &asset))
        throw std::runtime_error("invalid binary");

    auto material =
        CreateMaterial();

    ShaderPrefab* prefab = FindShader(asset.shader);
    if (prefab == nullptr)
        prefab = LoadShader(asset.shader);

    material->prefab = prefab;

    for (auto&& [name, buffer] : asset.overrides)
        material->Set(name, buffer.data(), buffer.size());

    for (auto&&[name, assetID] : asset.textures)
    {
    }

    s_AssetsManager->addInCache(id, material.handle);
    s_AssetsManager->mMaterialManager->addInCache(id, material.handle);

    return material;
}

AssetPtr<Material> AssetsManager::CreateMaterial()
{
    return s_AssetsManager->mMaterialManager->create();
}

Material& AssetsManager::GetMaterial(MaterialID h)
{
    return s_AssetsManager->mMaterialManager->get(h);
}

size_t AssetsManager::GetMaterialCount()
{
    return s_AssetsManager->mMaterialManager->GetCount();
}

usize AssetsManager::GetTextureCount()
{
    return s_AssetsManager->mTextureManager->count();
}

ShaderPrefab* AssetsManager::FindShader(AssetID id)
{
    return s_AssetsManager->mShaderPrefabManager->find(id);
}

ShaderPrefab* AssetsManager::LoadShader(AssetID id)
{
    if (auto prefab = FindShader(id))
        return prefab;

    auto info = AssetDB::GetAsset(id);
    if (info == nullptr)
        throw std::runtime_error("asset not found");

    auto path = AssetResolver::Resolve(info->uri) + HEADER_SHADER_EXTENSION;
    if (!gfile::exists(path))
        throw std::runtime_error("file not found");

    ShaderAsset asset;
    if (!ImportAssetFromFile(path, &asset))
        throw std::runtime_error("invalid binary");

    return s_AssetsManager->mShaderPrefabManager->create(id, &asset);
}

bool AssetsManager::ImportAssetFromFile(const std::string& file, IAsset* asset)
{
    FileStream stream(file, StreamType::INPUT);

    FileHeader header{};
    if (!stream.read(&header, sizeof(FileHeader)))
        return false;

    if (header.version != HEADER_ASSET_VERSION)
        return false;

    return asset->deserialize(&stream);
}

Handle AssetsManager::FindHandleInCache(AssetID id)
{
    return s_AssetsManager->findHandleInCache(id);
}

AssetID AssetsManager::FindAssetIDInCacheMaterial(Handle handle)
{
    return s_AssetsManager->mMaterialManager->findInCache(handle);
}

AssetID AssetsManager::FindAssetIDInCacheMesh(Handle handle)
{
    return s_AssetsManager->mMeshManager->findInCache(handle);
}

// private
AssetsManager::AssetsManager() : mMaterialManager(new MaterialManager()), mMeshManager(new MeshManager()), mShaderPrefabManager(new ShaderPrefabManager()), mTextureManager(new TextureManager())
{
    logger::info("[ AssetsManager ] Initialize %p.", this);
}

AssetsManager::~AssetsManager()
{
    logger::info("[ AssetsManager ] Shutdown %p.", this);

    delete mMaterialManager;

    delete mMeshManager;

    delete mShaderPrefabManager;

    delete mTextureManager;
}


void AssetsManager::addInCache(AssetID id, Handle handle)
{
    m_idToHandle[id] = handle;
}

Handle AssetsManager::findHandleInCache(AssetID id) const
{
    auto it = m_idToHandle.find(id);
    if (it != m_idToHandle.end())
        return it->second;
    return HANDLE_INVALID;
}

bool AssetsManager::LoadFromFile(const std::string& file, Hash64 sign, std::function<bool (Stream*)> func)
{
    FileStream stream(file, StreamType::INPUT);

    FileHeader header{};
    if (!stream.read(&header, sizeof(FileHeader)))
        return false;

    if (header.sign != sign)
        return false;

    return func(&stream);
}

void AssetsManager::loadGameObjectAsync(const std::string &name, std::function<void(Entity)> func)
{
    std::string file = AssetResolver::Resolve(URI::parse(name));

    loadGameObjectFromFileAsync(file + HEADER_MODEL_EXTENSION, func);
}

void AssetsManager::loadGameObjectFromFileAsync(const std::string &file, std::function<void(Entity)> func)
{
    auto scene = Scene::GetCurrent();
    if (scene != nullptr)
        TaskManager::Add<ModelLoadTask>(file, scene, func);
}





