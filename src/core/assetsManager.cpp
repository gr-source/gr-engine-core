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

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/core/filmbox/fbxAnimation.hpp"
#include "gr-base/core/filmbox/fbxScene.hpp"

#include <filesystem>
#include <cassert>
#include <future>

#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>
#include <gr-render/gError.h>

grm::u32 AssetsManager::m_processCount = 0;

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
std::unordered_map<std::string, std::unique_ptr<Animation>> AssetsManager::s_animationList;
std::list<std::tuple<std::string, Animation *>> AssetsManager::s_animationFromLoader;
std::mutex AssetsManager::s_animatioMutex;

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
    m_processCount = 0;

    /***************** Shader *****************/
    // mShaderMap.clear();
    // mShaderFromLoad.clear();

    /***************** Texture *****************/
    // /textureMap.clear();

    // multithread
    // m_mapHandleTexture.clear();
    // m_textureHandle.clear();

    /***************** Animation *****************/
    s_animationList.clear();
    s_animationFromLoader.clear();

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
    std::cout << "ee" << std::endl;


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
    mShaderMap.emplace(name, std::make_unique<grr::gShader>());

    auto shader = &(*mShaderMap[name]);

    mShaderList.push_back(shader);

    mShaderPathRef.emplace(shader, name);
    return shader;
}

std::string AssetsManager::GetShaderRefToName(grr::gShader *shader)
{
    auto it = mShaderPathRef.find(shader);
    if (it == mShaderPathRef.end()) {
        throw std::runtime_error("Shader do reference undefined.");
    }
    return it->second;
}

void AssetsManager::RemoveShader(grr::gShader *shader)
{
    auto it = mShaderPathRef.find(shader);
    if (it == mShaderPathRef.end())
    {
        return;
    }
    mShaderMap.erase(it->second);

    mShaderList.erase(std::remove(mShaderList.begin(), mShaderList.end(), it->first), mShaderList.end());

    mShaderPathRef.erase(it);
}

Animation *AssetsManager::LoadAnimation(const std::string &path)
{
    if (!File::Exists(path)) {
        Console::log(Log::ERROR, "Falied to open animation: " + path);
        return nullptr;
    }
    auto it = s_animationList.find(path);
    if (it != s_animationList.end()) {
        return &(*(*it).second);
    }

    {
        std::lock_guard<std::mutex> lock(s_animatioMutex);
        auto it = std::find_if(s_animationFromLoader.begin(), s_animationFromLoader.end(), [&path](auto &tuple) {
            return std::get<std::string>(tuple) == path;
        });
        if (it != s_animationFromLoader.end()) {
            return std::get<Animation *>((*it));
        }
    }

    auto animation = new Animation();
    {
        std::lock_guard<std::mutex> lock(s_animatioMutex);
        s_animationFromLoader.emplace_back(path, animation);
    }

    Threadpool::AddTask(&AssetsManager::ProcessAnimation);

    return animation;
}

grr::gShader *AssetsManager::FindShader(const std::string &name) {
    std::lock_guard<std::mutex> lock(mShaderMutex);
    auto it = mShaderMap.find(name);
    if (it == mShaderMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void AssetsManager::UpdateShader(const std::string &path, grr::gShader *shader)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    // check in list pre update
    {
        auto it = mShaderFromUpdate.find(path);
        if (it != mShaderFromUpdate.end())
        {
            return;
        }
    }

    mShaderFromUpdate.emplace(path, std::make_unique<ShaderTask>(path, nullptr, shader));
}

grr::gTexture *AssetsManager::CreateTexture(const std::string &name) {
    std::lock_guard<std::mutex> lock(mTextureMutex);
    mTextureMap.emplace(name, std::make_unique<grr::gTexture>());

    auto texture = &(*mTextureMap[name]);

    mTextureRefToName.emplace(texture, name);

    return texture;
}

// ********** Mesh ********** //
std::vector<Mesh> *AssetsManager::CreateMesh(const std::string &name) {
    std::lock_guard<std::mutex> lock(m_meshMutex);
    mMeshMapList.emplace(name, std::make_unique<std::vector<Mesh>>());
    
    auto ptr = &(*mMeshMapList[name]);

    mMeshListRefToName.emplace(ptr, name);
    return ptr;
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
    mShaderPropertyMap.emplace(shader, std::make_unique<ShaderProperty>());
    
    return &(*mShaderPropertyMap[shader]);
}

ShaderProperty *AssetsManager::GetShaderProperty(grr::gShader *shader)
{
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

void AssetsManager::SetupAnimation(const std::string &path, FbxAnimation *temp, Animation *animation) {
    animation->setName(temp->getName());
    animation->setDuration(temp->getDuration());
    animation->setTick(temp->getTick());

    for (const auto &channel : temp->getChannels()) {
        auto &keyframe = animation->addkeyframe(channel.getName(), channel.getName());

        for (auto &keyScale : channel.getKeysScale()) {
            keyframe.addKeyScale(keyScale.time, keyScale.value);
        }

        for (auto &keyRotation : channel.getKeysRotation()) {
            keyframe.addKeyRotation(keyRotation.time, keyRotation.value);
        }

        for (auto &keyPosition : channel.getKeysPosition()) {
            keyframe.addKeyPosition(keyPosition.time, keyPosition.value);
        }
    }

    s_animationList.emplace(path, std::unique_ptr<Animation>(animation));
    
    delete temp;
}

void AssetsManager::doAnimationInBackground(const std::string &path, Animation *target) {
    Stream stream(path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        return Console::LogError("Falied to open animation: %s", path.c_str());
    }

    FileHeader pHeader;
    stream.readS<FileHeader>(&pHeader, 1);

    gAnimationHeader pAnimationHeader;
    stream.readS<gAnimationHeader>(&pAnimationHeader, 1);

    auto fbxAnimation = new FbxAnimation(pAnimationHeader.m_name, pAnimationHeader.m_duration, pAnimationHeader.m_tick);

    grm::size sChannel;
    stream.readS<grm::size>(&sChannel, 1);

    std::vector<gChannel> pChannels(sChannel);

    for (auto i=0;i<sChannel;++i) {
        auto &pChannel = pChannels[i];

        stream.readS<char>(pChannel.m_name, 256);

        // gen key scale
        stream.readS<grm::size>(&pChannel.m_keyScaleCount, 1);
        pChannel.m_keyScale = new gKeyScale[pChannel.m_keyScaleCount];
        stream.readS<gKeyScale>(pChannel.m_keyScale, pChannel.m_keyScaleCount);

        //gen key rotation
        stream.readS<grm::size>(&pChannel.m_keyRotationCount, 1);
        pChannel.m_keyRotation = new gKeyRotation[pChannel.m_keyRotationCount];
        stream.readS<gKeyRotation>(pChannel.m_keyRotation, pChannel.m_keyRotationCount);

        // gen key position
        stream.readS<grm::size>(&pChannel.m_keyPositionCount, 1);
        pChannel.m_keyPosition = new gKeyPosition[pChannel.m_keyPositionCount];
        stream.readS<gKeyPosition>(pChannel.m_keyPosition, pChannel.m_keyPositionCount);
    }

    for (auto &channel : pChannels) {
        auto &keyframe = fbxAnimation->addChannel(channel.m_name);

        for (auto i=0;i<channel.m_keyScaleCount;++i) {
            auto &keyScale = channel.m_keyScale[i];

            keyframe.addKeyScale(keyScale.time, keyScale.value);
        }

        for (auto i=0;i<channel.m_keyRotationCount;++i) {
            auto &keyRotation = channel.m_keyRotation[i];

            keyframe.addKeyRotation(keyRotation.time, keyRotation.value);
        }

        for (auto i=0;i<channel.m_keyPositionCount;++i) {
            auto &keyPosition = channel.m_keyPosition[i];

            keyframe.addKeyPosition(keyPosition.time, keyPosition.value);
        }
    }

    // releaser memory
    for (auto i=0;i<sChannel;++i) {
        auto &pChannel = pChannels[i];

        // gen key scale
        delete[] pChannel.m_keyScale;

        //gen key rotation
        delete[] pChannel.m_keyRotation;

        // gen key position
        delete[] pChannel.m_keyPosition;
    }

    Event::Add(&AssetsManager::SetupAnimation, path, fbxAnimation, target);
}

void AssetsManager::ProcessAnimation() {
    std::unique_lock<std::mutex> lock(s_animatioMutex);
    while (!s_animationFromLoader.empty()) {
        auto &&[path, animation] = s_animationFromLoader.front();
        lock.unlock();

        doAnimationInBackground(path, animation);

        lock.lock();
        s_animationFromLoader.pop_front();
    }
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

// TODO: aicionar lista de callbacks a serem chamadas apos shader serem adicionado a thread
void AssetsManager::SetShaderList(const std::string &path, std::function<void(grr::gShader *)> func)
{
    std::lock_guard<std::mutex> lock(mShaderMutex);
    // check if contains in main list
    {
        auto it = mShaderMap.find(path);
        if (it != mShaderMap.end()) {
            if (func == nullptr)
                return;

            return func(&(*it->second));
        }
    }

    // check there loading
    {
        auto it = mShaderTaskMap.find(path);
        if (it != mShaderTaskMap.end()) {
            if (func == nullptr)
                return;

            return it->second->addFunction(func);
        }
    }

    // check file exists
    if (!File::Exists(path))
    {
        return Console::LogError("[ AssetsManager-Shader ] File do not exists: " + path);
    }

    mShaderTaskMap.emplace(path, std::make_unique<ShaderTask>(path, func));
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

grm::size AssetsManager::GetShaderCount() {
    return mShaderMap.size();
}

grm::size AssetsManager::GetMeshCount() {
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









