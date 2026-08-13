#pragma once

#include <gr-render/gRender.h>

#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/asset/textureAsset.hpp"
#include "gr-base/asset/default.hpp"

#include "gr-base/core/assetPtr.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/default.hpp"

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
    static AssetPtr<gr::texture> LoadTexture(const std::string& uri);

    static AssetPtr<gr::texture> LoadTexture(AssetID id);

    static AssetPtr<gr::texture> CreateTexture();

    static void DestroyTexture(AssetPtr<gr::texture> ptr);

    static gr::texture* GetTexture(TextureHandle h);

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
    // static MeshHandle LoadMesh(const std::string& name);

    static AssetPtr<Mesh> LoadMesh(AssetID id);

    static size_t GetMeshCount();

    // API Privada
    static AssetPtr<Mesh> CreateMesh();

    static void RemoveMesh(AssetPtr<Mesh> ptr);

    static Mesh *GetMesh(MeshHandle h);

    // shader property
    static usize GetTextureCount();

    // ********** Material API publica ********** //
    // static MaterialHandle LoadMaterial(const std::string& name);

    static AssetPtr<Material> LoadMaterial(AssetID id);

    static AssetPtr<Material> CreateMaterial();

    // ********** Material API privada ********** //
    static Material *GetMaterial(MaterialHandle h);

    static size_t GetMaterialCount();

    // static ShaderPrefab* FindShader(const std::string &name);

    static ShaderPrefab* FindShader(AssetID id);

    // static ShaderPrefab* LoadShader(const std::string& name);

    static ShaderPrefab* LoadShader(AssetID id);

    static bool ImportAssetFromFile(const std::string& file, IAsset* asset);

    static Material LoadMaterialFromFile(const std::string& file);

    static Handle FindHandleInCache(AssetID id);

    static AssetID FindAssetIDInCacheMaterial(Handle handle);

    static AssetID FindAssetIDInCacheMesh(Handle handle);

private:
    static AssetsManager* s_AssetsManager;

    // Cache
    std::unordered_map<AssetID, Handle> m_idToHandle;

    ShaderPrefabManager *mShaderPrefabManager;

    // fields
    MaterialManager* mMaterialManager;

    TextureManager* mTextureManager;

    // ShaderManager mShaderManager;

    MeshManager* mMeshManager;

    AssetsManager();
    ~AssetsManager();

    void addInCache(AssetID id, Handle handle);

    Handle findHandleInCache(AssetID id) const;

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



