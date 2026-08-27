#pragma once

#include <gr-render/gRender.h>

// #include "gr-base/asset/shaderasset.hpp"
// #include "gr-base/asset/textureAsset.hpp"
#include "gr-base/asset/default.hpp"

#include "gr-base/core/assetPtr.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/texture.hpp"
#include "gr-base/managers/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/prefab/default.hpp"

#include <functional>

#include <gr-render/default.hpp>

class AssetsManager
{
public:
    static void Initialize();

    static void Shutdown();

    // ********** Texture ********** //
    static TextureRef LoadTexture(const std::string& uri);

    static TextureRef LoadTexture(AssetID id);

    static TextureRef CreateTexture(const TextureDesc& desc);

    static void DestroyTexture(TextureRef ptr);

    static gr::texture* GetTexture(TextureID h);

    // template <typename ...Args>
    // static void LoadTexture(const std::string &path, Args &&...args);

    // static std::string GetRefTextureName(gr::gTexture *texture);

    // template <typename ...Args>
    // static std::shared_ptr<TaskInfo> LoadShaderAsync(ShaderParams shaderParams, Args &&...args);

    // static gr::Shader *CreateShader(const std::string& name);

    // static gr::Shader *FindShader(const std::string &name);

    // static std::string GetShaderName(gr::Shader *shader);

    // static void RemoveShader(gr::Shader *shader);

    template <typename ...Args>
    static void LoadGameObjectAsync(const std::string &name, Args &&...args);

    // ********** Mesh ********** //
    // static MeshID LoadMesh(const std::string& name);

    static MeshRef LoadMesh(AssetID id);

    static size_t GetMeshCount();

    // API Privada
    static MeshRef CreateMesh();

    static void RemoveMesh(MeshRef ptr);

    static Mesh *GetMesh(MeshID h);

    // shader property
    static usize GetTextureCount();

    // ********** Material API publica ********** //
    // static MaterialID LoadMaterial(const std::string& name);

    static MaterialRef LoadMaterial(AssetID id);

    static MaterialRef CreateMaterial();

    // ********** Material API privada ********** //
    static Material* GetMaterial(MaterialID h);

    static size_t GetMaterialCount();

    // static ShaderPrefab* FindShader(const std::string &name);

    static ShaderPrefab* FindShader(AssetID id);

    // static ShaderPrefab* LoadShader(const std::string& name);

    static ShaderPrefab* LoadShader(AssetID id);

    static bool ImportAssetFromFile(const std::string& file, IAsset* asset);

    // static Handle FindHandleInCache(AssetID id);

    static AssetID FindAssetIDInCacheMaterial(MaterialID materialID);

    static AssetID FindAssetIDInCacheMesh(MeshID meshID);

private:
    static AssetsManager* s_AssetsManager;

    // Cache
    std::unordered_map<AssetID, TextureID> textureIDCache;
    std::unordered_map<AssetID, MaterialID> materialIDCache;

    std::unordered_map<AssetID, MeshID> meshIDToCache;

    ShaderPrefabManager *mShaderPrefabManager;

    // fields
    MaterialManager* mMaterialManager;

    TextureManager* mTextureManager;

    // ShaderManager mShaderManager;

    MeshManager* mMeshManager;

    AssetsManager();
    ~AssetsManager();

    void addMeshIDInCache(AssetID id, MeshID meshID);

    void addTextureIDInCache(AssetID id, TextureID textureID);

    void addMaterialIDInCache(AssetID id, MaterialID materialID);

    MeshID findMeshIDInCache(AssetID id) const;

    TextureID findTextureIDInCache(AssetID id) const;

    MaterialID findMaterialIDInCache(AssetID id) const;

    static bool LoadFromFile(const std::string& file, Hash64 sign, std::function<bool (Stream*)> func);

    void loadGameObjectAsync(const std::string &name, std::function<void(Entity)> func);

    void loadGameObjectFromFileAsync(const std::string &file, std::function<void(Entity)> func);
};

template <typename... Args>
inline void AssetsManager::LoadGameObjectAsync(const std::string &name, Args &&...args)
{
    std::function<void(Entity)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    s_AssetsManager->loadGameObjectAsync(name, func);
}



