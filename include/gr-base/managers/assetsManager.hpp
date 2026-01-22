#pragma once

#include <gr-render/gRender.h>
#include <memory>

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/task/materialmanager.hpp"
#include "gr-base/core/task/scenetaskmanager.hpp"
#include "gr-base/core/task/texturemanager.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/task/animationmanager.hpp"
#include "gr-base/core/task/shadermanager.hpp"

#include "gr-base/managers/meshmanager.hpp"

class AssetsManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename ...Args>
    static void LoadTexture(const std::string &path, Args &&...args);

    static std::string GetRefTextureName(grr::gTexture *texture);

    template <typename ...Args>
    static std::shared_ptr<TaskInfo<grr::gShader*>> LoadShaderAsync(ShaderParams shaderParams, Args &&...args);

    template <typename ...Args>
    static std::shared_ptr<TaskInfo<grr::gShader*>> LoadShader(ShaderParams shaderParams, grr::gShader *shader, Args &&...args);

    static grr::gShader *CreateShader(ShaderParams shaderParams);

    static grr::gShader *FindShader(const std::string &name);

    static std::string GetShaderName(grr::gShader *shader);

    static void RemoveShader(grr::gShader *shader);

    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    template <typename ...Args>
    static std::shared_ptr<TaskInfo<int>> LoadScene(const std::string &path, Args &&...args);

    template <typename ...Args>
    static std::shared_ptr<TaskInfo<int>> SceneSaveAsync(const std::string &path, const Scene* scene, Args &&...args);

    /*
    template <typename ...Args>
    static void LoadAnimation(const std::string &path, Args &&...args);

    static Animation *CreateAnimation(const std::string &name);

    static Animation *FindAnimation(const std::string &name);

    static std::string GetAnimationName(const Animation *animation);
    */

    // ********** Texture ********** //
    static grr::gTexture *CreateTexture(const std::string &name);

    // ********** Mesh ********** //
    static MeshID CreateMesh(const std::string &name);

    static Mesh *GetMesh(MeshID id);

    static MeshID FindMesh(const std::string &name);

    static std::string GetMeshName(MeshID id);

    static void RemoveMesh(MeshID id);
    
    // shader property
    static ShaderProperty *CreateShaderProperty(grr::gShader *shader);

    static ShaderProperty *GetShaderProperty(grr::gShader *shader);

    static usize GetTextureCount();

    static size_t GetMeshCount();

    static void SetGameObjectList(const std::string &path, std::function<void(GameObjectID)> func);

    static std::vector<grr::gShader *> GetShaderList(ShaderType type);

    static MaterialID CreateMaterial(const std::string &name);

    static Material &GetMaterial(MaterialID id);

    static MaterialID FindMaterial(const std::string &name);

    static std::string GetMaterialName(MaterialID id);

    static void RemoveTask(const std::string &name);

private:
    static AssetsManager* s_AssetsManager;

    // static AnimationManager mAnimationManager;

    // sshader property
    static std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> mShaderPropertyMap;

    // fields
    SceneTaskManager mSceneTaskManager;

    MaterialManager mMaterialManager;

    TextureManager mTextureManager;

    ShaderManager mShaderManager;

    MeshManager mMeshManager;

    AssetsManager() = default;
    ~AssetsManager() = default;
};

template <typename... Args>
inline void AssetsManager::LoadGameObject(const std::string &path, Args &&...args)
{
    std::function<void(GameObjectID)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    SetGameObjectList(path, func);
}

template <typename... Args>
inline std::shared_ptr<TaskInfo<int>> AssetsManager::LoadScene(const std::string &path, Args &&...args)
{
    std::function<void (Scene *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    return s_AssetsManager->mSceneTaskManager.create_task(path, func);
}

template <typename... Args>
inline std::shared_ptr<TaskInfo<int>> AssetsManager::SceneSaveAsync(const std::string &path, const Scene* scene, Args &&...args)
{
    std::function<void ()> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)...);

    return s_AssetsManager->mSceneTaskManager.create_save_task(path, scene, func);
}

template <typename ...Args>
inline void AssetsManager::LoadTexture(const std::string &path, Args &&...args) {
    std::function<void(grr::gTexture *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    return s_AssetsManager->mTextureManager.load(path, func);
}

// ***** Shader ***** //
template <typename ...Args>
inline std::shared_ptr<TaskInfo<grr::gShader*>> AssetsManager::LoadShaderAsync(ShaderParams shaderParams, Args &&...args)
{
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    
    return s_AssetsManager->mShaderManager.load(shaderParams, nullptr, func);
}

template <typename... Args>
inline std::shared_ptr<TaskInfo<grr::gShader*>> AssetsManager::LoadShader(ShaderParams shaderParams, grr::gShader *shader, Args &&...args)
{
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    
    return s_AssetsManager->mShaderManager.load(shaderParams, shader, func);
}

/*
template <typename ...Args>
void AssetsManager::LoadAnimation(const std::string &path, Args &&...args)
{
    return mAnimationManager.load(path, std::forward<Args>(args)...);
}
*/



