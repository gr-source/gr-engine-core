#pragma once

#include <gr-render/gRender.h>

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/task/texturemanager.hpp"

#include "gr-base/managers/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/prefab/default.hpp"

class AssetsManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename ...Args>
    static void LoadTexture(const std::string &path, Args &&...args);

    static std::string GetRefTextureName(gr::gTexture *texture);

    // template <typename ...Args>
    // static std::shared_ptr<TaskInfo> LoadShaderAsync(ShaderParams shaderParams, Args &&...args);

    // static gr::Shader *CreateShader(const std::string& name);

    // static gr::Shader *FindShader(const std::string &name);

    // static std::string GetShaderName(gr::Shader *shader);

    // static void RemoveShader(gr::Shader *shader);

    template <typename ...Args>
    static void LoadGameObject(const std::string &name, Args &&...args);

    template <typename ...Args>
    static void LoadGameObjectFromFile(const std::string &file, Args &&...args);

    // ********** Texture ********** //
    static gr::gTexture *CreateTexture(const std::string &name);

    // ********** Mesh ********** //
    static MeshHandle LoadMesh(const std::string& name);

    static MeshHandle LoadMesh(AssetHash hash);

    static size_t GetMeshCount();

    // API Privada
    static Mesh LoadMeshFromFile(const std::string& file);

    static MeshHandle CreateMesh(AssetHash hash = INVALID_HASH64);

    static MeshHandle FindMesh(AssetHash hash);

    static void RemoveMesh(MeshHandle handle);

    static Mesh *GetMesh(MeshHandle h);

    // shader property
    static usize GetTextureCount();

    static const std::vector<ShaderPrefab*>& GetAllShader3D();

    // ********** Material API publica ********** //
    static MaterialHandle LoadMaterial(const std::string& name);

    static MaterialHandle LoadMaterial(AssetHash hash);

    static MaterialHandle CreateMaterial(AssetHash hash = INVALID_HASH64);

    // ********** Material API privada ********** //
    static Material LoadMaterialFromFile(const std::string& file);

    static MaterialHandle FindMaterial(AssetHash hash);

    static Material *GetMaterial(MaterialHandle h);

    static size_t GetMaterialCount();

    static void RemoveTask(const std::string &name);

    static ShaderPrefab* FindShader(const std::string &name);

    static ShaderPrefab* FindShader(Hash64 hash);

    static ShaderPrefab* LoadShader(const std::string& name);

    static ShaderPrefab* LoadShader(Hash64 hash);

    static ShaderPrefab* LoadShaderFromFile(const std::string& file);

private:
    static AssetsManager* s_AssetsManager;

    ShaderPrefabManager *mShaderPrefabManager;

    // fields
    MaterialManager* mMaterialManager;

    TextureManager mTextureManager;

    // ShaderManager mShaderManager;

    MeshManager* mMeshManager;

    std::string mAssetDir;

    AssetsManager();
    ~AssetsManager();

    static bool LoadFromFile(const std::string& file, Hash64 sign, std::function<bool (Stream*)> func);

    void loadGameObject(const std::string &name, std::function<void(Entity)> func);

    void loadGameObjectFromFile(const std::string &file, std::function<void(Entity)> func);
};

template <typename... Args>
inline void AssetsManager::LoadGameObject(const std::string &name, Args &&...args)
{
    std::function<void(Entity)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    s_AssetsManager->loadGameObject(name, func);
}

template <typename... Args>
inline void AssetsManager::LoadGameObjectFromFile(const std::string &file, Args &&...args)
{
    std::function<void(Entity)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    s_AssetsManager->loadGameObjectFromFile(file, func);
}

template <typename ...Args>
inline void AssetsManager::LoadTexture(const std::string &path, Args &&...args) {
    std::function<void(gr::gTexture *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    return s_AssetsManager->mTextureManager.load(path, func);
}





