#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include <cassert>
#include <functional>

#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>
#include <gr-render/gError.h>
#include <memory>
#include <mutex>

#include <algorithm>

/***************** Shader *****************/
std::unordered_map<std::string, std::unique_ptr<ShaderTask>> AssetsManager::mShaderTaskMap;
std::unordered_map<std::string, std::unique_ptr<grr::gShader>> AssetsManager::mShaderMap;
std::unordered_map<grr::gShader *, std::string> AssetsManager::mShaderPathRef;
std::vector<grr::gShader *> AssetsManager::mShaderList;
std::mutex AssetsManager::mShaderMutex;

// shader update
std::unordered_map<std::string, std::unique_ptr<ShaderTask>> AssetsManager::mShaderFromUpdate;

// sshader property
std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> AssetsManager::mShaderPropertyMap;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> AssetsManager::mTextureMap;
std::unordered_map<grr::gTexture *, std::string> AssetsManager::mTextureRefToName;

std::unordered_map<std::string, std::unique_ptr<TextureTask>> AssetsManager::mTextureTaskMap;
std::mutex AssetsManager::mTextureMutex;

/***************** Animation *****************/
std::unordered_map<std::string, std::unique_ptr<AnimationLoadTask>> AssetsManager::mAnimationLoadTask;
std::unordered_map<std::string, std::unique_ptr<Animation>> AssetsManager::mAnimationList;
std::mutex AssetsManager::mAnimationMutex;

/***************** GameObject *****************/
std::unordered_map<std::string, std::unique_ptr<GameObjectTask>> AssetsManager::mGameObjectTaskMap;
std::mutex AssetsManager::mGameObjectTaskMutex;

/***************** Mesh *****************/
std::unordered_map<std::string, std::unique_ptr<std::vector<Mesh>>> AssetsManager::mMeshMapList;
std::unordered_map<std::vector<Mesh> *, std::string> AssetsManager::mMeshListRefToName;
std::mutex AssetsManager::m_meshMutex;

/***************** Material *****************/
std::unordered_map<std::string, std::unique_ptr<Material>> AssetsManager::mMaterialMap;
std::unordered_map<Material *, std::string> AssetsManager::mMaterialRefToName;
std::mutex AssetsManager::mMaterialMutex;

/***************** Scene *****************/
std::unordered_map<std::string, std::unique_ptr<SceneTask>> AssetsManager::mSceneTaskMap;
std::unordered_map<std::string, std::unique_ptr<Scene>> AssetsManager::mSceneMap;
std::mutex AssetsManager::mSceneMutex;

#include "gr-base/core/stb/stb_image.h"

void AssetsManager::Initialize() {
    stbi_set_flip_vertically_on_load(true);
}

// TODO: Falta corrigir
void AssetsManager::Shutdown() {
    // clear assets

    /***************** Shader *****************/
    // mShaderMap.clear();
    // mShaderFromLoad.clear();

    /***************** Texture *****************/
    // /textureMap.clear();

    // multithread
    // m_mapHandleTexture.clear();
    // m_textureHandle.clear();

    /***************** Animation *****************/
    /*s_animationList.clear();*/
    /*s_animationFromLoader.clear();*/

    /***************** GameObject *****************/
    // s_gameObjectFromLoader.clear();

    /***************** Mesh *****************/
    // s_meshList.clear();
}

#include "gr-base/core/window.hpp"
#include <imgui.h>

void AssetsManager::Render() {
    auto bCreatMenu = !mSceneTaskMap.empty();
    if (!bCreatMenu)
    {
        return;
    }

    auto mainWindowSize = Window::GetWindowSize() / 2;
    auto beginSize = ImVec2{400, 300};

    ImGui::SetNextWindowSize(beginSize);
    ImGui::SetNextWindowPos(ImVec2{static_cast<float>(mainWindowSize.x), static_cast<float>(mainWindowSize.y)} - (beginSize / 2.0f));

    ImGui::Begin("TaskList", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings);

    for (auto &&[name, sceneTask] : mSceneTaskMap)
    {
        sceneTask->render();
    }

    ImGui::End();
}

std::string AssetsManager::GetRefTextureName(grr::gTexture *texture) {
    std::lock_guard<std::mutex> lock(mTextureMutex);
    auto it = mTextureRefToName.find(texture);
    if (it == mTextureRefToName.end()) {
        throw std::runtime_error("texture has not reference.");
    }
    return it->second;
}

grr::gShader *AssetsManager::CreateShader(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    auto &&[it, badded] = mShaderMap.emplace(name, std::make_unique<grr::gShader>());

    mShaderList.push_back(&(*it->second));

    mShaderPathRef.emplace(&(*it->second), name);

    return &(*it->second);
}

std::string AssetsManager::GetShaderRefToName(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    auto it = mShaderPathRef.find(shader);
    if (it == mShaderPathRef.end()) {
        throw std::runtime_error("Shader do reference undefined.");
    }
    return it->second;
}

void AssetsManager::RemoveShader(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    auto it = mShaderPathRef.find(shader);
    if (it == mShaderPathRef.end())
    {
        return;
    }
    mShaderMap.erase(it->second);

    mShaderList.erase(std::remove(mShaderList.begin(), mShaderList.end(), it->first), mShaderList.end());

    mShaderPathRef.erase(it);
}

grr::gShader *AssetsManager::FindShader(const std::string &name) {
    std::lock_guard<std::mutex> lock(mShaderMutex);
    auto it = mShaderMap.find(name);
    if (it == mShaderMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

grr::gTexture *AssetsManager::CreateTexture(const std::string &name) {
    std::lock_guard<std::mutex> lock(mTextureMutex);
    auto &&[it, badded] = mTextureMap.emplace(name, std::make_unique<grr::gTexture>());

    mTextureRefToName.emplace(&(*it->second), name);

    return &(*it->second);
}

// ********** Mesh ********** //
std::vector<Mesh> *AssetsManager::CreateMesh(const std::string &name) {
    std::lock_guard<std::mutex> lock(m_meshMutex);
    auto &&[it, badded] = mMeshMapList.emplace(name, std::make_unique<std::vector<Mesh>>());
    
    mMeshListRefToName.emplace(&(*it->second), name);

    return &(*it->second);
}

std::vector<Mesh> *AssetsManager::FindMesh(const std::string &name) {
    std::lock_guard<std::mutex> lock(m_meshMutex);
    auto it = mMeshMapList.find(name);
    if (it == mMeshMapList.end()) {
        return nullptr;
    }
    return &(*it->second);
}

std::string AssetsManager::GetMeshListRefToName(std::vector<Mesh> *meshList)
{
    auto it = mMeshListRefToName.find(meshList);
    if (it == mMeshListRefToName.end())
    {
        throw std::runtime_error("MeshList do not register.");
    }
    return it->second;
}

void AssetsManager::RemoveMesh(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_meshMutex);
    auto it = mMeshMapList.find(name);
    if (it == mMeshMapList.end()) {
        return;
    }
    mMeshListRefToName.erase(&(*it->second));

    mMeshMapList.erase(it);
}

ShaderProperty *AssetsManager::CreateShaderProperty(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    mShaderPropertyMap.emplace(shader, std::make_unique<ShaderProperty>());
    
    return &(*mShaderPropertyMap[shader]);
}

ShaderProperty *AssetsManager::GetShaderProperty(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    auto it = mShaderPropertyMap.find(shader);
    if (it == mShaderPropertyMap.end())
    {
        return nullptr;
    }
    return &(*it->second);
}

std::unordered_map<std::string, std::unique_ptr<std::vector<Mesh>>> &AssetsManager::GetMeshListMap()
{
    return mMeshMapList;
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func)
{
    if (!File::Exists(path))
    {
        return Console::LogError("[ AssetsManager::GameObject ] File not found: %s", path.c_str());
    }

    std::lock_guard<std::mutex> lock(mGameObjectTaskMutex);
    // check has be loading
    {
        auto it = mGameObjectTaskMap.find(path);
        if (it != mGameObjectTaskMap.end()) {
            return;
        }
    }

    mGameObjectTaskMap.emplace(path, std::make_unique<GameObjectTask>(path, func));
}

void AssetsManager::SetShaderList(const std::string &path, grr::gShader *shader, std::function<void(grr::gShader *)> func)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    // check there loading
    {
        auto it = mShaderTaskMap.find(path);
        if (it != mShaderTaskMap.end())
        {
            if (func == nullptr)
                return;

            return it->second->addFunction(func);
        }
    }

    // check if contains in main list
    {
        auto it = mShaderMap.find(path);
        if (it != mShaderMap.end() && shader == nullptr)
        {
            if (func == nullptr)
                return;

            return func(&(*it->second));
        }
    }

    mShaderTaskMap.emplace(path, std::make_unique<ShaderTask>(path, func, shader));
}

void AssetsManager::SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func)
{
    std::lock_guard<std::mutex> lock(mTextureMutex);

    // Check texture has in main list
    {
        auto it = mTextureMap.find(path);
        if (it != mTextureMap.end())
        {
            if (func == nullptr)
                return;
            return func(&(*it->second));
        }
    }

    // Check texture is loading
    {
        auto it = mTextureTaskMap.find(path);
        if (it != mTextureTaskMap.end())
        {
            if (func == nullptr)
                return;
            return it->second->addFunction(func);
        }
    }

    if (!File::Exists(path))
    {
        return Console::LogError("[ AssetsManager::Texture ] File not found: %s", path.c_str());
    }
    
    // Create task to load texture
    mTextureTaskMap.emplace(path, std::make_unique<TextureTask>(path, func));
}

void AssetsManager::SetAnimationList(const std::string &path, const std::function<void (Animation *)> func)
{
    std::lock_guard<std::mutex> lock(mAnimationMutex);
    
    // check task is processing
    {
        auto it = mAnimationLoadTask.find(path);
        if (it != mAnimationLoadTask.end())
        {
            if (func == nullptr)
                return;

            return it->second->addFunction(func);
        }
    }

    // check animation has be loaded
    {
        auto it = mAnimationList.find(path);
        if (it != mAnimationList.end())
        {
            if (func == nullptr)
                return;
            
            return func(&(*it->second));
        }
    }

    if (!File::Exists(path))
    {
        return Console::LogError("[ AssetsManager::Animation ] Fail to load, file do not found: %s", path.c_str());
    }

    mAnimationLoadTask.emplace(path, std::make_unique<AnimationLoadTask>(path, func));
}

Animation *AssetsManager::CreateAnimation(const std::string &name)
{
    std::lock_guard<std::mutex> lock(mAnimationMutex);

    auto &&[it, badded] = mAnimationList.emplace(name, std::make_unique<Animation>(name));

    return &(*it->second);
}

std::unordered_map<std::string, std::unique_ptr<grr::gShader>> &AssetsManager::GetShaderMap() {
    return mShaderMap;
}

std::vector<grr::gShader *> AssetsManager::GetShaderList()
{
    return mShaderList;
}

grm::size AssetsManager::GetTextureCount() {
    std::lock_guard<std::mutex> lock(mTextureMutex);
    return mTextureMap.size();
}

grm::size AssetsManager::GetShaderCount()
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    return mShaderMap.size();
}

grm::size AssetsManager::GetMeshCount()
{
    std::lock_guard<std::mutex> lock(m_meshMutex);
    return mMeshMapList.size();
}

std::unordered_map<grr::gTexture *, std::string> AssetsManager::GetTextureRefMap()
{
    std::lock_guard<std::mutex> lock(mTextureMutex);
    return mTextureRefToName;
}

Material *AssetsManager::FindMaterial(const std::string &key) {
    std::lock_guard<std::mutex> lock(mMaterialMutex);
    auto it = mMaterialMap.find(key);
    if (it == mMaterialMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

std::string AssetsManager::GetRefMaterialToName(Material *material) {
    std::lock_guard<std::mutex> lock(mMaterialMutex);
    auto it = mMaterialRefToName.find(material);
    if (it == mMaterialRefToName.end()) {
        return "";
    }
    return it->second;
}

std::unordered_map<Material *, std::string> AssetsManager::GetMaterialRefMap() {
    return mMaterialRefToName;
}

void AssetsManager::RemoveTask(const std::string &name)
{
    // Check if task on the scene
    {
        std::lock_guard<std::mutex> lock(mSceneMutex);
        auto it = mSceneTaskMap.find(name);
        if (it != mSceneTaskMap.end())
        {
            mSceneTaskMap.erase(it);
            return;
        }
    }

    // Check if task on the texture
    {
        std::lock_guard<std::mutex> lock(mTextureMutex);
        auto it = mTextureTaskMap.find(name);
        if (it != mTextureTaskMap.end())
        {
            mTextureTaskMap.erase(it);
            return;
        }
    }

    // Check in to shader task
    {
        std::lock_guard<std::mutex> lock(mShaderMutex);
        // shader update
        {
            auto it = mShaderFromUpdate.find(name);
            if (it != mShaderFromUpdate.end())
            {
                mShaderFromUpdate.erase(it);
                return;
            }
        }
        
        // shader from load
        {
            auto it = mShaderTaskMap.find(name);
            if (it != mShaderTaskMap.end())
            {
                mShaderTaskMap.erase(it);
                return;
            }
        }
    }

    // Check in task of gameobject
    {
        std::lock_guard<std::mutex> lock(mGameObjectTaskMutex);
        auto it = mGameObjectTaskMap.find(name);
        if (it != mGameObjectTaskMap.end()) {
            mGameObjectTaskMap.erase(it);
            return;
        }
    }

    // Chck task is on AnimationLoadTask
    {
        std::lock_guard<std::mutex> lock(mAnimationMutex);
        auto it = mAnimationLoadTask.find(name);
        if (it != mAnimationLoadTask.end())
        {
            mAnimationLoadTask.erase(it);
            return;
        }
    }
}

void AssetsManager::SetSceneList(const std::string &path, SceneLoadFlags flags, std::function<void(Scene *)> func)
{
    std::lock_guard<std::mutex> lock(mSceneMutex);
    // Check in list has bee loaded
    {
        auto it = mSceneMap.find(path);
        if (it != mSceneMap.end())
        {
            if (func == nullptr)
                return;
            return func(&(*it->second));
        }
    }

    // Check in list of task
    {
        auto it = mSceneTaskMap.find(path);
        if (it != mSceneTaskMap.end())
        {
            if (func == nullptr)
                return;
            return it->second->addFunction(func);
        }
    }

    if (!File::Exists(path))
    {
        return Console::LogError("[ AssetsManager ] Scene.LoadScene: file '%s' do not found.", path.c_str());
    }

    mSceneTaskMap.emplace(path, std::make_unique<SceneTask>(path, flags, func));
}









