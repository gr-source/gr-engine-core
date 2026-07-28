#pragma once

#include <gr-render/gRender.h>

#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/asset/textureAsset.hpp"
#include "gr-base/asset/default.hpp"

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

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
    static TextureHandle LoadTexture(const std::string& name);

    static TextureHandle LoadTexture(AssetHash hash);

    static TextureHandle CreateTexture(AssetHash hash = INVALID_HASH64);

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

    static MeshHandle LoadMesh(AssetID id);

    static size_t GetMeshCount();

    // API Privada
    static MeshHandle CreateMesh();

    static MeshHandle FindMesh(AssetID id);

    static void RemoveMesh(MeshHandle handle);

    static Mesh *GetMesh(MeshHandle h);

    // shader property
    static usize GetTextureCount();

    // ********** Material API publica ********** //
    // static MaterialHandle LoadMaterial(const std::string& name);

    static MaterialHandle LoadMaterial(AssetID id);

    static MaterialHandle CreateMaterial();

    // ********** Material API privada ********** //
    static MaterialHandle FindMaterial(AssetID id);

    static Material *GetMaterial(MaterialHandle h);

    static size_t GetMaterialCount();

    // static ShaderPrefab* FindShader(const std::string &name);

    static ShaderPrefab* FindShader(AssetID id);

    // static ShaderPrefab* LoadShader(const std::string& name);

    static ShaderPrefab* LoadShader(AssetID id);

    static ShaderAsset ImportShaderAssetFromFile(const std::string& file);

    static TextureAsset ImportTextureAssetFromFile(const std::string& file);

    static Material LoadMaterialFromFile(const std::string& file);

    static Mesh LoadMeshFromFile(const std::string& file);

private:
    static AssetsManager* s_AssetsManager;

    std::unordered_map<AssetID, Handle> m_hashmap;

    ShaderPrefabManager *mShaderPrefabManager;

    // fields
    MaterialManager* mMaterialManager;

    TextureManager* mTextureManager;

    // ShaderManager mShaderManager;

    MeshManager* mMeshManager;

    AssetsManager();
    ~AssetsManager();

    Handle findInCache(AssetID id) const;

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



