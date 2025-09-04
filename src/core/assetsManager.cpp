#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/shader.hpp"

#include "gr-base/core/task/runtime/gameobjectloadtask.hpp"
#include "gr-base/core/task/scenetaskmanager.hpp"
#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include <cassert>
#include <functional>

#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>
#include <gr-render/gError.h>
#include <memory>
#include <mutex>

AnimationManager AssetsManager::mAnimationManager;

// sshader property
std::unordered_map<grr::gShader *, std::unique_ptr<ShaderProperty>> AssetsManager::mShaderPropertyMap;

#include "gr-base/core/stb/stb_image.h"

void AssetsManager::Initialize() {
    stbi_set_flip_vertically_on_load(true);
}

// TODO: Falta corrigir
void AssetsManager::Shutdown()
{
}

#include <imgui.h>

void AssetsManager::Render()
{
}

std::string AssetsManager::GetRefTextureName(grr::gTexture *texture)
{
    auto name = GetTextureManager().getname(texture);
    if (name == nullptr)
        throw std::runtime_error("texture has not reference.");
    return name;
}

grr::gShader *AssetsManager::CreateShader(ShaderParams shaderParams)
{
    return GetShaderManager().create(shaderParams);
}

grr::gShader *AssetsManager::FindShader(const std::string &name)
{
    return GetShaderManager().find(name);
}

std::string AssetsManager::GetShaderName(grr::gShader *shader)
{
    auto name = GetShaderManager().get_shader_name(shader);
    if (name == nullptr)
    {
        throw std::runtime_error("[ AssetsManager.Shader ] Fail to get shader name, pointer is not valid.");
    }
    return name;
}

void AssetsManager::RemoveShader(grr::gShader *shader)
{
    return GetShaderManager().destroy(shader);
}

grr::gTexture *AssetsManager::CreateTexture(const std::string &name)
{
    return GetTextureManager().create(name);
}

// ********** Mesh ********** //
MeshID AssetsManager::CreateMesh(const std::string &name)
{
    return GetMeshManager().create(name);
}

Mesh &AssetsManager::GetMesh(MeshID id)
{
    return GetMeshManager().get(id);
}

MeshID AssetsManager::FindMesh(const std::string &name)
{
    return GetMeshManager().find(name);
}

std::string AssetsManager::GetMeshName(MeshID id)
{
    auto name = GetMeshManager().getname(id);
    if (name == nullptr)
        throw std::runtime_error("MeshList do not register.");
    return name;
}

void AssetsManager::RemoveMesh(MeshID id)
{
    return GetMeshManager().remove(id);
}

ShaderProperty *AssetsManager::CreateShaderProperty(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(GetShaderManager().s_Mutex);
    mShaderPropertyMap.emplace(shader, std::make_unique<ShaderProperty>());
    
    return &(*mShaderPropertyMap[shader]);
}

ShaderProperty *AssetsManager::GetShaderProperty(grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(GetShaderManager().s_Mutex);
    auto it = mShaderPropertyMap.find(shader);
    if (it == mShaderPropertyMap.end())
    {
        return nullptr;
    }
    return &(*it->second);
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func)
{
    if (!File::Exists(path))
    {
        return Console::LogError("[ AssetsManager::GameObject ] File not found: %s", path.c_str());
    }

    TaskManager::Add<GameObjectLoadTask>(path, func);
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
*/

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

/*
std::vector<grr::gShader *> AssetsManager::GetShaderList(AssetShader_ type, ShaderOptimize_ optimize)
{
    return GetShaderManager().get_shader(type, optimize);
}
*/

std::vector<grr::gShader *> AssetsManager::GetShaderList(ShaderType type)
{
    return GetShaderManager().get_shader(type);
}

MaterialID AssetsManager::CreateMaterial(const std::string &name, grr::gShader *shader)
{
    return GetMaterialManager().create(shader, name);
}

Material &AssetsManager::GetMaterial(MaterialID id)
{
    return GetMaterialManager().get(id);
}

grm::size AssetsManager::GetTextureCount()
{
    return GetTextureManager().getcount();
}

std::size_t AssetsManager::GetMeshCount()
{
    return GetMeshManager().getcount();
}

MaterialID AssetsManager::FindMaterial(const std::string &name)
{
    return GetMaterialManager().find(name);
}

std::string AssetsManager::GetMaterialName(MaterialID id)
{
    return GetMaterialManager().getname(id);
}

void AssetsManager::RemoveTask(const std::string &name)
{
    // Check if task on the scene
    if (GetSceneTaskManager().remove_task(name))
        return;

    // Check if task on the texture
    if (GetTextureManager().remove_task(name))
        return;

    if (GetShaderManager().remove_task(name))
        return;

    // Chck task is on AnimationLoadTask
    if (mAnimationManager.remove_task(name))
        return;
    /*
    if (GetFilmboxManager().remove_task(name))
    {
        return;
    }
    */
}

// private
/*
FilmboxManager &AssetsManager::GetFilmboxManager()
{
    static FilmboxManager instance;
    return instance;
}
*/
SceneTaskManager &AssetsManager::GetSceneTaskManager()
{
    static SceneTaskManager instance;
    return instance;
}

MaterialManager &AssetsManager::GetMaterialManager()
{
    static MaterialManager instance;
    return instance;
}

TextureManager &AssetsManager::GetTextureManager()
{
    static TextureManager instance;
    return instance;
}

ShaderManager &AssetsManager::GetShaderManager()
{
    static ShaderManager instance;
    return instance;
}

MeshManager &AssetsManager::GetMeshManager()
{
    static MeshManager instance;
    return instance;
}








