#pragma once

#include <gr-render/gRender.h>
#include <memory>

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/task/scenetaskmanager.hpp"
#include "gr-base/core/task/texturemanager.hpp"

// #include "gr-base/managers/shadermanager.hpp"

#include "gr-base/managers/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/asset/default.hpp"
#include "gr-base/asset/asset.hpp"

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

    template <typename ...Args>
    static std::shared_ptr<TaskInfo> LoadScene(const std::string &path, Args &&...args);

    template <typename ...Args>
    static std::shared_ptr<TaskInfo> SceneSaveAsync(const std::string &path, const Scene* scene, Args &&...args);

    /*
    template <typename ...Args>
    static void LoadAnimation(const std::string &path, Args &&...args);

    static Animation *CreateAnimation(const std::string &name);

    static Animation *FindAnimation(const std::string &name);

    static std::string GetAnimationName(const Animation *animation);
    */

    // ********** Texture ********** //
    static gr::gTexture *CreateTexture(const std::string &name);

    // ********** Mesh ********** //
    static MeshID CreateMesh(const std::string &name, const MeshAsset* src = nullptr);

    static Mesh *GetMesh(MeshID id);

    static MeshID FindMesh(const std::string &name);

    static std::string GetMeshName(MeshID id);

    static void RemoveMesh(MeshID id);
    
    // shader property
    static usize GetTextureCount();

    static size_t GetMeshCount();

    static MaterialID GetMaterialCount();

    static const std::vector<ShaderPrefab*>& GetAllShader3D();

    static MaterialID CreateMaterial(const std::string &name);

    static Material *GetMaterial(MaterialID id);

    static MaterialID FindMaterial(const std::string &name);

    static std::string GetMaterialName(MaterialID id);

    static void RemoveTask(const std::string &name);

    static MaterialID LoadMaterial(const std::string& name);

    static MaterialID LoadMaterialFromFile(const std::string& file);

    static ShaderPrefab* LoadShader2(const std::string& name);

    static ShaderPrefab* LoadShaderFromFile(const std::string& file);

    static MeshID LoadMesh(const std::string& name);

    static MeshID LoadMeshFromFile(const std::string& file);

private:
    static AssetsManager* s_AssetsManager;

    ShaderPrefabManager *mShaderPrefabManager;

    // fields
    SceneTaskManager mSceneTaskManager;

    MaterialManager* mMaterialManager;

    TextureManager mTextureManager;

    // ShaderManager mShaderManager;

    MeshManager* mMeshManager;

    std::string mAssetDir;

    AssetsManager();
    ~AssetsManager();

    static bool LoadFromFile(const std::string& file, Asset* asset);

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

template <typename... Args>
inline std::shared_ptr<TaskInfo> AssetsManager::LoadScene(const std::string &path, Args &&...args)
{
    std::function<void (Scene *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    return s_AssetsManager->mSceneTaskManager.create_task(path, func);
}

template <typename... Args>
inline std::shared_ptr<TaskInfo> AssetsManager::SceneSaveAsync(const std::string &path, const Scene* scene, Args &&...args)
{
    std::function<void ()> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)...);

    return s_AssetsManager->mSceneTaskManager.create_save_task(path, scene, func);
}

template <typename ...Args>
inline void AssetsManager::LoadTexture(const std::string &path, Args &&...args) {
    std::function<void(gr::gTexture *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    return s_AssetsManager->mTextureManager.load(path, func);
}

/*
// ***** Shader ***** //
template <typename ...Args>
inline std::shared_ptr<TaskInfo> AssetsManager::LoadShaderAsync(ShaderParams shaderParams, Args &&...args)
{
    std::function<void(gr::Shader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    
    return s_AssetsManager->mShaderManager.load(shaderParams, func);
}
*/

/*
template <typename ...Args>
void AssetsManager::LoadAnimation(const std::string &path, Args &&...args)
{
    return mAnimationManager.load(path, std::forward<Args>(args)...);
}
*/




