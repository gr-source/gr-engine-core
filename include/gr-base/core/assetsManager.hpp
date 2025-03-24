#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <mutex>
#include <list>

#include <gr-render/gRender.h>

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/filmbox/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/task/animationloadtask.hpp"
#include "gr-base/core/task/gameObjectTask.hpp"
#include "gr-base/core/task/textureTask.hpp"
#include "gr-base/core/task/shaderTask.hpp"
#include "gr-base/core/task/sceneTask.hpp"

class AssetsManager {
public:
    static void Initialize();

    static void Shutdown();

    static void Render();

    template <typename ...Args>
    static void LoadTexture(const std::string &path, Args &&...args);

    static std::string GetRefTextureName(grr::gTexture *texture);

    template <typename ...Args>
    static void LoadShader(const std::string &path, Args &&...args);

    static grr::gShader *CreateShader(const std::string &name);

    static std::string GetShaderRefToName(grr::gShader *shader);

    static void RemoveShader(grr::gShader *shader);

    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadScene(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadAnimation(const std::string &path, Args &&...args);

    static Animation *CreateAnimation(const std::string &name);

    static grr::gShader *FindShader(const std::string &name);

    template <typename ...Args>
    static void UpdateShader(const std::string &path, grr::gShader *shader, Args &&...args);

    // ********** Texture ********** //
    static grr::gTexture *CreateTexture(const std::string &name);

    // ********** Mesh ********** //
    static std::vector<Mesh> *CreateMesh(const std::string &name);

    static std::vector<Mesh> *FindMesh(const std::string &name);

    static std::string GetMeshListRefToName(std::vector<Mesh> *meshList);

    static void RemoveMesh(const std::string &name);
    
    // shader property
    static ShaderProperty *CreateShaderProperty(grr::gShader *shader);

    static ShaderProperty *GetShaderProperty(grr::gShader *shader);

    static std::unordered_map<std::string, std::unique_ptr<std::vector<Mesh>>> &GetMeshListMap();

    static grm::u32 m_processCount;

    static grm::size GetTextureCount();

    static grm::size GetShaderCount();
    
    static grm::size GetMeshCount();

    static std::unordered_map<grr::gTexture *, std::string> GetTextureRefMap();

    static void SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func);

    static void SetShaderList(const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func);

    static void SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func);

    static void SetAnimationList(const std::string &path, std::function<void(Animation *)> func);

    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> &GetShaderMap();

    static std::vector<grr::gShader *> GetShaderList();

    template <typename ...Args>
    static Material *CreateMaterial(const std::string &key, Args &&...args);

    static Material *FindMaterial(const std::string &key);

    static std::string GetRefMaterialToName(Material *material);

    static std::unordered_map<Material *, std::string> GetMaterialRefMap();

    static void RemoveTask(const std::string &name);

private:
    /***************** Shader *****************/
    static std::unordered_map<std::string, std::unique_ptr<ShaderTask>> mShaderTaskMap;
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> mShaderMap;
    static std::unordered_map<grr::gShader *, std::string> mShaderPathRef;
    static std::vector<grr::gShader *> mShaderList;
    static std::mutex mShaderMutex;

    // Update Shader
    static std::unordered_map<std::string, std::unique_ptr<ShaderTask>> mShaderFromUpdate;

    // sshader property
    static std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> mShaderPropertyMap;

    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> mTextureMap;
    static std::unordered_map<grr::gTexture *, std::string> mTextureRefToName;

    // multithread
    static std::unordered_map<std::string, std::unique_ptr<TextureTask>> mTextureTaskMap;
    static std::mutex mTextureMutex;

    /***************** Animation *****************/
    static std::unordered_map<std::string, std::unique_ptr<AnimationLoadTask>> mAnimationLoadTask;
    static std::unordered_map<std::string, std::unique_ptr<Animation>> mAnimationList;
    static std::mutex mAnimationMutex;

    /***************** GameObject *****************/
    static std::unordered_map<std::string, std::unique_ptr<GameObjectTask>> mGameObjectTaskMap;
    static std::mutex mGameObjectTaskMutex;

    /***************** Mesh *****************/
    static std::unordered_map<std::string, std::unique_ptr<std::vector<Mesh>>> mMeshMapList;
    static std::unordered_map<std::vector<Mesh> *, std::string> mMeshListRefToName;
    static std::mutex m_meshMutex;
    
    /***************** Material *****************/
    static std::unordered_map<std::string, std::unique_ptr<Material>> mMaterialMap;
    static std::unordered_map<Material *, std::string> mMaterialRefToName;
    static std::mutex mMaterialMutex;

    /***************** Scene *****************/
    static std::unordered_map<std::string, std::unique_ptr<SceneTask>> mSceneTaskMap;
    static std::unordered_map<std::string, std::unique_ptr<Scene>> mSceneMap;
    static std::mutex mSceneMutex;

    static void SetSceneList(const std::string &path, SceneLoadFlags flags, std::function<void (Scene *)> func);
};

template <typename... Args>
inline void AssetsManager::LoadGameObject(const std::string &path, Args &&...args) {
    std::function<void(GameObject *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    SetGameObjectList(path, func);
}

template <typename... Args>
inline void AssetsManager::LoadScene(const std::string &path, Args &&...args) {
    std::function<void (Scene *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    SetSceneList(path, SceneLoadFlags_Load, func);
}

template <typename ...Args>
inline void AssetsManager::LoadTexture(const std::string &path, Args &&...args) {
    std::function<void(grr::gTexture *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    SetTextureList(path, func);
}

// ***** Shader ***** //
template <typename... Args>
inline void AssetsManager::LoadShader(const std::string &path, Args &&...args) {
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    
    SetShaderList(path, nullptr, func);
}

template <typename ...Args>
inline void AssetsManager::UpdateShader(const std::string &path, grr::gShader *shader, Args &&...args)
{
    std::function<void (grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    SetShaderList(path, shader, func);
}

// ***** Material ***** //
template <typename... Args>
inline Material *AssetsManager::CreateMaterial(const std::string &key, Args && ...args) {
    std::lock_guard<std::mutex> lock(mMaterialMutex);
    auto it = mMaterialMap.find(key);
    if (it != mMaterialMap.end()) {
        return &(*it->second);
    }
    mMaterialMap[key] = std::make_unique<Material>(std::forward<Args>(args)...);

    mMaterialRefToName[&(*mMaterialMap[key])] = key;

    return &(*mMaterialMap[key]);
}

template <typename ...Args>
void AssetsManager::LoadAnimation(const std::string &path, Args &&...args)
{
    std::function<void (Animation *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    SetAnimationList(path, func);
}



