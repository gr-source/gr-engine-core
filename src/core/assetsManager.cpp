#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <cassert>
#include <functional>

#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>
#include <gr-render/gError.h>
#include <memory>
#include <mutex>

/***************** Shader *****************/
ShaderManager AssetsManager::mShaderManager;

// sshader property
std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> AssetsManager::mShaderPropertyMap;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> AssetsManager::mTextureMap;
std::unordered_map<grr::gTexture *, std::string> AssetsManager::mTextureRefToName;

std::unordered_map<std::string, std::unique_ptr<TextureLoadTask>> AssetsManager::mTextureLoadTaskMap;
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
void AssetsManager::Shutdown()
{
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

grr::gShader *AssetsManager::CreateShader(AssetShader type, ShaderOptimize_ optmize, const std::string &name)
{
    return dynamic_cast<grr::gShader *>(mShaderManager.create(type, optmize, name));
}

grr::gShader *AssetsManager::FindShader(ShaderOptimize optmize, const std::string &name)
{
    return dynamic_cast<grr::gShader *>(mShaderManager.find(optmize, name));
}

std::string AssetsManager::GetShaderName(grr::gShader *shader)
{
    auto name = mShaderManager.get_shader_name(static_cast<const Shader *>(shader));
    if (name == nullptr)
    {
        throw std::runtime_error("[ AssetsManager.Shader ] Fail to get shader name, pointer is not valid.");
    }
    return name;
}

void AssetsManager::RemoveShader(grr::gShader *shader)
{
    std::cout << "Remove: " << GetShaderName(shader) << std::endl;
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
    std::lock_guard<std::mutex> lock(mShaderManager.mutex);
    mShaderPropertyMap.emplace(shader, std::make_unique<ShaderProperty>());
    
    return &(*mShaderPropertyMap[shader]);
}

ShaderProperty *AssetsManager::GetShaderProperty(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(mShaderManager.mutex);
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
        auto it = mTextureLoadTaskMap.find(path);
        if (it != mTextureLoadTaskMap.end())
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
    mTextureLoadTaskMap.emplace(path, std::make_unique<TextureLoadTask>(path, func));
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

std::vector<Shader *> AssetsManager::GetShaderList(AssetShader type, ShaderOptimize_ optimize)
{
    return mShaderManager.get_shader(type, optimize);
}

std::vector<Shader *> AssetsManager::GetShaderList(AssetShader type)
{
    return mShaderManager.get_shader(type);
}

grm::size AssetsManager::GetTextureCount() {
    std::lock_guard<std::mutex> lock(mTextureMutex);
    return mTextureMap.size();
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
        auto it = mTextureLoadTaskMap.find(name);
        if (it != mTextureLoadTaskMap.end())
        {
            mTextureLoadTaskMap.erase(it);
            return;
        }
    }

    mShaderManager.remove_task(name);

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









