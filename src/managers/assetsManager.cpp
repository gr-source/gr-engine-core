#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/shader.hpp"

#include "gr-base/core/task/runtime/gameobjectloadtask.hpp"
#include "gr-base/core/task/scenetaskmanager.hpp"
#include "gr-base/core/thread/taskmanager.hpp"

#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>
#include <gr-render/gError.h>

AssetsManager* AssetsManager::s_AssetsManager = nullptr;

// AnimationManager AssetsManager::mAnimationManager;

// sshader property
std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> AssetsManager::mShaderPropertyMap;

void AssetsManager::Initialize()
{
    if (s_AssetsManager == nullptr)
        s_AssetsManager = new AssetsManager();
}

void AssetsManager::Shutdown()
{
    if (s_AssetsManager != nullptr)
        delete s_AssetsManager;

    s_AssetsManager = nullptr;
}

std::string AssetsManager::GetRefTextureName(grr::gTexture *texture)
{
    auto name = s_AssetsManager->mTextureManager.getname(texture);
    if (name == nullptr)
        throw std::runtime_error("texture has not reference.");
    return name;
}

grr::gShader *AssetsManager::CreateShader(ShaderParams shaderParams)
{
    return s_AssetsManager->mShaderManager.create(shaderParams);
}

grr::gShader *AssetsManager::FindShader(const std::string &name)
{
    return s_AssetsManager->mShaderManager.find(name);
}

std::string AssetsManager::GetShaderName(grr::gShader *shader)
{
    auto name = s_AssetsManager->mShaderManager.get_shader_name(shader);
    if (name == nullptr)
        throw std::runtime_error("[ AssetsManager.Shader ] Fail to get shader name, pointer is not valid.");
    return name;
}

void AssetsManager::RemoveShader(grr::gShader *shader)
{
    return s_AssetsManager->mShaderManager.destroy(shader);
}

grr::gTexture *AssetsManager::CreateTexture(const std::string &name)
{
    return s_AssetsManager->mTextureManager.create(name);
}

// ********** Mesh ********** //
MeshID AssetsManager::CreateMesh(const std::string &name)
{
    return s_AssetsManager->mMeshManager.create(name);
}

Mesh &AssetsManager::GetMesh(MeshID id)
{
    return s_AssetsManager->mMeshManager.get(id);
}

MeshID AssetsManager::FindMesh(const std::string &name)
{
    return s_AssetsManager->mMeshManager.find(name);
}

std::string AssetsManager::GetMeshName(MeshID id)
{
    auto name = s_AssetsManager->mMeshManager.getname(id);
    if (name == nullptr)
        throw std::runtime_error("MeshList do not register.");
    return name;
}

void AssetsManager::RemoveMesh(MeshID id)
{
    return s_AssetsManager->mMeshManager.remove(id);
}

ShaderProperty *AssetsManager::CreateShaderProperty(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(s_AssetsManager->mShaderManager.s_Mutex);
    mShaderPropertyMap.emplace(shader, std::make_unique<ShaderProperty>());
    
    return &(*mShaderPropertyMap[shader]);
}

ShaderProperty *AssetsManager::GetShaderProperty(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(s_AssetsManager->mShaderManager.s_Mutex);
    auto it = mShaderPropertyMap.find(shader);
    if (it == mShaderPropertyMap.end())
    {
        return nullptr;
    }
    return &(*it->second);
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObjectID)> func)
{
    TaskManager::Add<GameObjectID, GameObjectLoadTask>(path, func);
}

/*
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
    return mAnimationManager.create(name);
}

Animation *AssetsManager::FindAnimation(const std::string &name)
{
    return mAnimationManager.find(name);
}

std::string AssetsManager::GetAnimationName(const Animation *animation)
{
    auto name = mAnimationManager.get_name(animation);
    return name == nullptr ? "" : name;
}
*/
/*
std::vector<grr::gShader *> AssetsManager::GetShaderList(AssetShader_ type, ShaderOptimize_ optimize)
{
    return s_AssetsManager->mShaderManager.get_shader(type, optimize);
}
*/

std::vector<grr::gShader *> AssetsManager::GetShaderList(ShaderType type)
{
    return s_AssetsManager->mShaderManager.get_shader(type);
}

MaterialID AssetsManager::CreateMaterial(const std::string &name, grr::gShader *shader)
{
    return s_AssetsManager->mMaterialManager.create(shader, name);
}

Material &AssetsManager::GetMaterial(MaterialID id)
{
    return s_AssetsManager->mMaterialManager.get(id);
}

usize AssetsManager::GetTextureCount()
{
    return s_AssetsManager->mTextureManager.getcount();
}

std::size_t AssetsManager::GetMeshCount()
{
    return s_AssetsManager->mMeshManager.getcount();
}

MaterialID AssetsManager::FindMaterial(const std::string &name)
{
    return s_AssetsManager->mMaterialManager.find(name);
}

std::string AssetsManager::GetMaterialName(MaterialID id)
{
    return s_AssetsManager->mMaterialManager.getname(id);
}

void AssetsManager::RemoveTask(const std::string &name)
{
    // Check if task on the scene
    if (s_AssetsManager->mSceneTaskManager.remove_task(name))
        return;

    // Check if task on the texture
    if (s_AssetsManager->mTextureManager.remove_task(name))
        return;

    if (s_AssetsManager->mShaderManager.remove_task(name))
        return;
}







