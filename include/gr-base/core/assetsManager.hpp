#pragma once

#include <gr-render/gRender.h>
#include <utility>

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/task/animationmanager.hpp"
#include "gr-base/core/task/shadermanager.hpp"

#include "gr-base/core/task/runtime/gameobjectloadtask.hpp"
#include "gr-base/core/task/runtime/textureloadtask.hpp"
#include "gr-base/core/task/runtime/sceneloadtask.hpp"

#include "gr-base/core/task/filmboxmanager.hpp"

class AssetsManager
{
public:
    static void Initialize();

    static void Shutdown();

    static void Render();

    template <typename ...Args>
    static void LoadTexture(const std::string &path, Args &&...args);

    static std::string GetRefTextureName(grr::gTexture *texture);

    template <typename ...Args>
    static std::shared_ptr<TaskInfo> LoadShader(ShaderParams shaderParams, const std::string &path, Args &&...args);

    template <typename ...Args>
    static std::shared_ptr<TaskInfo> LoadShader(ShaderParams shaderParams, const std::string &path, grr::gShader *shader, Args &&...args);

    static grr::gShader *CreateShader(ShaderParams shaderParams);

    static grr::gShader *FindShader(ShaderOptimize optmize, const std::string &name);

    static std::string GetShaderName(grr::gShader *shader);

    static void RemoveShader(grr::gShader *shader);

    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadScene(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadAnimation(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadFilmbox(const std::string &path, Args &&...args);

    static Animation *CreateAnimation(const std::string &name);

    static Animation *FindAnimation(const std::string &name);

    static std::string GetAnimationName(const Animation *animation);

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

    static grm::size GetTextureCount();

    static grm::size GetMeshCount();

    static std::unordered_map<grr::gTexture *, std::string> GetTextureRefMap();

    static void SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func);

    static void SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func);

    // static std::vector<grr::gShader *> GetShaderList(AssetShader_ type, ShaderOptimize_ optimize);

    // static std::vector<grr::gShader *> GetShaderList(AssetShader_ type);

    template <typename ...Args>
    static Material *CreateMaterial(const std::string &key, Args &&...args);

    static Material *FindMaterial(const std::string &key);

    static std::string GetRefMaterialToName(Material *material);

    static std::unordered_map<Material *, std::string> GetMaterialRefMap();

    static void RemoveTask(const std::string &name);

private:
    static AnimationManager mAnimationManager;

    static FilmboxManager mFilmboxManager;

    // sshader property
    static std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> mShaderPropertyMap;

    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> mTextureMap;
    static std::unordered_map<grr::gTexture *, std::string> mTextureRefToName;

    // multithread
    static std::unordered_map<std::string, std::unique_ptr<TextureLoadTask>> mTextureLoadTaskMap;
    static std::mutex mTextureMutex;

    /***************** GameObject *****************/
    static std::unordered_map<std::string, std::unique_ptr<GameObjectLoadTask>> mGameObjectTaskMap;
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
    static std::unordered_map<std::string, std::unique_ptr<SceneLoadTask>> mSceneTaskMap;
    static std::unordered_map<std::string, std::unique_ptr<Scene>> mSceneMap;
    static std::mutex mSceneMutex;

    
    static ShaderManager &GetShaderManager();

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
template <typename ...Args>
inline std::shared_ptr<TaskInfo> AssetsManager::LoadShader(ShaderParams shaderParams, const std::string &path, Args &&...args)
{
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    
    return GetShaderManager().create_shader_task_load(shaderParams, path, nullptr, func);
}

template <typename... Args>
inline std::shared_ptr<TaskInfo> AssetsManager::LoadShader(ShaderParams shaderParams, const std::string &path, grr::gShader *shader, Args &&...args)
{
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0)
    {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    
    return GetShaderManager().create_shader_task_load(shaderParams, path, shader, func);
}

// ***** Material ***** //
template <typename... Args>
inline Material *AssetsManager::CreateMaterial(const std::string &key, Args && ...args) {
    std::lock_guard<std::mutex> lock(mMaterialMutex);
    mMaterialMap.emplace(key, std::make_unique<Material>(std::forward<Args>(args)...));

    mMaterialRefToName.emplace(&(*mMaterialMap[key]), key);

    return &(*mMaterialMap[key]);
}

template <typename ...Args>
void AssetsManager::LoadAnimation(const std::string &path, Args &&...args)
{
    return mAnimationManager.load(path, std::forward<Args>(args)...);
}

template <typename ...Args>
void AssetsManager::LoadFilmbox(const std::string &path, Args &&...args)
{
    return mFilmboxManager.load(path, std::forward<Args>(args)...);
}



