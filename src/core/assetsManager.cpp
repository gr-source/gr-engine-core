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

#define STB_IMAGE_IMPLEMENTATION
#include "gr-base/core/stb/stb_image.h"

grm::u32 AssetsManager::m_processCount = 0;

/***************** Shader *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gShader>> AssetsManager::s_shaderList;
std::list<ShaderHandle> AssetsManager::s_shaderFromLoader;
std::mutex AssetsManager::s_shaderMutex;

/***************** Texture *****************/
std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> AssetsManager::textureMap;
std::unordered_map<grr::gTexture *, std::string> AssetsManager::textureRefToName;

std::unordered_map<std::string, TextureHandle *> AssetsManager::m_mapHandleTexture;
std::list<TextureHandle> AssetsManager::m_textureHandle;
std::mutex AssetsManager::m_mutexTexture;

/***************** Animation *****************/
std::unordered_map<std::string, std::unique_ptr<Animation>> AssetsManager::s_animationList;
std::list<std::tuple<std::string, Animation *>> AssetsManager::s_animationFromLoader;
std::mutex AssetsManager::s_animatioMutex;

/***************** GameObject *****************/
std::list<GameObjectHandle> AssetsManager::s_gameObjectFromLoader;
std::mutex AssetsManager::s_gameObjectMutex;

/***************** Mesh *****************/
std::unordered_map<std::string, std::unique_ptr<Mesh>> AssetsManager::s_meshList;
std::mutex AssetsManager::m_meshMutex;

/***************** Material *****************/
std::unordered_map<std::string, std::unique_ptr<Material>> AssetsManager::materialsMap;
std::unordered_map<Material *, std::string> AssetsManager::materialRefToName;
std::mutex AssetsManager::materialsMutex;

void AssetsManager::Initialize() {
    stbi_set_flip_vertically_on_load(true);
}

// TODO: Falta corrigir
void AssetsManager::Shutdown() {
    // clear assets
    m_processCount = 0;

    /***************** Shader *****************/
    s_shaderList.clear();
    s_shaderFromLoader.clear();

    /***************** Texture *****************/
    textureMap.clear();

    // multithread
    m_mapHandleTexture.clear();
    m_textureHandle.clear();

    /***************** Animation *****************/
    s_animationList.clear();
    s_animationFromLoader.clear();

    /***************** GameObject *****************/
    s_gameObjectFromLoader.clear();

    /***************** Mesh *****************/
    s_meshList.clear();
}

std::string AssetsManager::GetRefTextureName(grr::gTexture *texture) {
    auto it = textureRefToName.find(texture);
    if (it == textureRefToName.end()) {
        return "";
    }
    return it->second;
}

Animation *AssetsManager::LoadAnimation(const std::string &path) {
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
    auto it = s_shaderList.find(name);
    if (it == s_shaderList.end()) {
        return nullptr;
    }
    return &(*it->second);
}

// ********** Mesh ********** //

Mesh *AssetsManager::FindMesh(const std::string &name) {
    std::lock_guard<std::mutex> lock(m_meshMutex);
    auto it = s_meshList.find(name);
    if (it == s_meshList.end()) {
        return nullptr;
    }
    return &(*it->second);
}

void AssetsManager::DeleteMesh(Mesh *target) {
    std::lock_guard<std::mutex> lock(m_meshMutex);
    auto it = s_meshList.find(target->getName());
    if (it == s_meshList.end()) {
        return;
    }
    s_meshList.erase(it);
}

std::unordered_map<std::string, std::unique_ptr<Mesh>> &AssetsManager::GetMeshListMap() {
    return s_meshList;
}

/***************** Texture Process ****************/
void AssetsManager::SetupTexture(TextureHandle handle, int width, int height, int ch, void *pixels) {
    std::unique_lock<std::mutex> lock(m_mutexTexture);
    auto it = textureMap.find(handle.m_path);
    if (it != textureMap.end()) {
        return handle.invoke(&(*it->second));
    }
    auto texture = grr::gTexture::Create();

    grr::TextureFormat format;

    {

        switch (ch) {
        case 1:
            // return Texture::eRED;
        case 2:
            // return Texture::eRG;
        case 3:
            format = grr::TextureFormat_SRGB;
            break;
        case 4:
            format = grr::TextureFormat_SRGBA;
            break;
        default:
            break;
        }

        grr::gTexture::Bind(texture);
        grr::gTexture::UpdateTexture(width, height, grr::gTextureFlags_MipMaps | grr::gTextureFlags_Filter_Bilinear, format, pixels);

        stbi_image_free(pixels);
    }

    auto &&[ita, added] = textureMap.emplace(handle.m_path, std::unique_ptr<grr::gTexture>(texture));
    textureRefToName[&(*ita->second)] = handle.m_path;

    handle.invoke(texture);

    lock.unlock();

    Console::LogInfo("[ AssetsManager::Texture ] Texture loaded: " + handle.m_path);
}

void AssetsManager::ProcessTexture() {
    std::unique_lock<std::mutex> lock(m_mutexTexture);
    while (!m_textureHandle.empty()) {
        auto handle = m_textureHandle.front();
        m_textureHandle.pop_front();

        auto path = handle.m_path;
        m_mapHandleTexture.erase(path);

        int width, height, ch;
        auto pixels = reinterpret_cast<void *>(stbi_load(path.c_str(), &width, &height, &ch, 0));
        if (!pixels) {
            Console::LogError("[ AssetsManager::Texture ] Failed to load texture: " + path);
            handle.invoke(nullptr);
            continue;
        }
        lock.unlock();

        Event::Add(&AssetsManager::SetupTexture, handle, width, height, ch, pixels);

        lock.lock();
    }
}

/***************** Shader Process ****************/
void AssetsManager::SetupShader(ShaderHandle handle, const std::string &frag, const std::string &vert) {
    auto it = s_shaderList.find(handle.m_name);
    if (it != s_shaderList.end()) {
        handle.invoke(&(*it->second));
        return;
    }

    const char *fShader = frag.c_str();
    const char *vShader = vert.c_str();

    auto shader = new grr::gShader();

    shader->build(&fShader, 1, &vShader, 1);
    if (!shader->isValid()) {
        Console::LogError("[ Shader ] Fail to build shader, %s, %s", handle.m_name.c_str(), grr::error());
        delete shader;
        return;
    }
    
    s_shaderList.emplace(handle.m_name, std::unique_ptr<grr::gShader>(shader));

    handle.invoke(shader);

    Console::LogInfo("[ AssetsManager::Shader ] Shader loadedd sucessfull: %s", handle.m_name.c_str());
}

void AssetsManager::ProcessShader() {
    std::unique_lock<std::mutex> lock(s_shaderMutex);
    while (!s_shaderFromLoader.empty()) {
        auto handle = s_shaderFromLoader.front();
        s_shaderFromLoader.pop_front();
        lock.unlock();

        // Processing shader
        auto fragSource = File::GetFileContent(handle.m_frag);
        if (fragSource.empty()) {
            Console::log(Log::ERROR, "Falied to shader: " + handle.m_name + " Fragment: " + handle.m_frag);

            lock.lock();
            continue;
        }

        // Loading vertex shader
        auto vertSource = File::GetFileContent(handle.m_vert);
        if (vertSource.empty()) {
            Console::log(Log::ERROR, "Falied to shader: " + handle.m_name + " Vertex: " + handle.m_vert);

            lock.lock();
            continue;
        }

        // lock.unlock();
        Event::Add(AssetsManager::SetupShader, handle, fragSource, vertSource);

        lock.lock();
    }
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

void AssetsManager::doInBackgroundGameObject(GameObjectHandle handle) {
    Stream stream(handle.m_path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        return Console::LogError("[ Model3DHandler ] Falied to open file: %s", handle.m_path.c_str());
    }

    FileHeader header;
    if (!stream.readS(&header, 1)) {
        return Console::LogError("[ Model3DHandler ] Falied to read header binary.");
    }
    
    if (header.signature[0] != 'g' && header.signature[1] != 'r' && header.signature[2] != 'm' && header.signature[3] != 'x') {
        return Console::LogError("[ Model3DHandler ] Invalid binary type.");
    }

    Console::LogInfo("[ Model3DHandler ] GRMX loading model.");

    ReadData(&stream, handle);

    Console::LogInfo("[ Model3DHandler ] Scene loaded sucess full.");
}

void AssetsManager::ReadData(Stream *stream, GameObjectHandle handle) {
    GameObject *gameObject = nullptr;
    std::unordered_map<std::string, GameObject *> m_gameObjectTemp;

    while (true) {
        DataSegment segment;
        if (!stream->readS(&segment, 1)) {
            break;
        }

        if (segment.type == gHeaderType_None && segment.sizeData == 0) {
            break;
        }

        auto segment_name = struct_name[segment.type];

        auto next_position = stream->tellg() + static_cast<std::streampos>(segment.sizeData);

        auto result = -1;
        switch (segment.type) {
        case gHeaderType_Object:
            result = ReadGameObject(stream, nullptr, &gameObject, m_gameObjectTemp);
            break;
        case gHeaderType_Texture:
            result = ReadTexture(stream, m_gameObjectTemp);
            break;
        case gHeaderType_Material:
            result = ReadMaterial(stream, m_gameObjectTemp);
            break;
        case gHeaderType_Script:
            result = ReadScript(stream, m_gameObjectTemp);
            break;
        default:
            break;
        }

        if (result != 0) {
            Console::LogError("[ AssetsManager ] Falied to load segment: %s code: %d", struct_name[segment.type], result);
        }

        if (stream->tellg() != next_position) {
            stream->seekg(next_position, std::ios_base::beg);

            Console::LogWarning("[ AssetsManager ] Block position is invalid");
        }
    }

    Event::Add(handle.m_function, gameObject);
}

int AssetsManager::ReadMaterial(Stream *stream, std::unordered_map<std::string, GameObject *> &gameObjectMap) {
    std::size_t sname = 0;
    if (!stream->read(reinterpret_cast<char *>(&sname), sizeof(std::size_t))) {
        return 1;
    }

    std::string name(sname, '\0');
    if (!stream->read(name.data(), sname * sizeof(char))) {
        return 2;
    }

    if (gameObjectMap.find(name) == gameObjectMap.end()) {
        std::cout << name << std::endl;
        return 3;
    }

    auto go = gameObjectMap[name];

    std::size_t mcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&mcount), sizeof(std::size_t))) {
        return 4;
    }

    for (std::size_t i=0;i<mcount;i++) {
        std::size_t mnsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&mnsize), sizeof(std::size_t))) {
            return 5;
        }

        std::string mname(mnsize, '\0');
        if (!stream->read(mname.data(), mnsize * sizeof(char))) {
            return 6;
        }

        std::size_t bufferSize = 0;
        if (!stream->read(reinterpret_cast<char *>(&bufferSize), sizeof(std::size_t))) {
            return 7;
        }

        auto next = stream->tellg() + static_cast<std::streampos>(bufferSize);
        
        if (AssetsManager::FindMaterial(mname) == nullptr) {
            auto material = AssetsManager::CreateMaterial(mname, mname);
            material->deserialize(stream);
        }

        if (stream->tellg() != next) {
            stream->seekg(next, std::ios_base::beg);
        }
    }

    std::size_t ccount = 0;
    if (!stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 8;
    }

    int result = 0;
    for (std::size_t i=0;i<ccount;i++) {
        result = ReadMaterial(stream, gameObjectMap);
        if (result != 0) {
            break;
        }
    }

    return result;
}

void AssetsManager::CreateComponent(GameObject *gameObject, std::string componentName) {
    auto entt = gameObject->getEntity();

    auto component = entt->hasComponent(componentName) ? entt->getComponent(componentName) : entt->addComponent(componentName);
    if (!component) {
        return Console::LogError("[ Assetsmanager.GameObject ] Falied to serialize component: %s, %s - Invalid componnet.", gameObject->getName().c_str(), componentName.c_str());
    }
}

void AssetsManager::SetupScript(GameObject *gameObject, std::string path, std::streampos position, std::string componentName) {
    auto entt = gameObject->getEntity();

    auto pool = ComponentRegistry::Find(componentName);
    if (!pool) {
        return Console::LogError("[ AssetsManager ] Falied to addcomponent %s, component not found.", componentName.c_str());
    }

    auto component = entt->hasComponent(componentName) ? entt->getComponent(componentName) : entt->addComponent(componentName);
    if (!component) {
        return Console::LogError("[ Assetsmanager.GameObject ] Falied to serialize component: %s, %s - Invalid componnet.", gameObject->getName().c_str(), componentName.c_str());
    }

    Stream stream(path, std::ios_base::in | std::ios_base::out);
    stream.seekg(position, std::ios_base::beg);

    pool->deserialize(component, &stream);
}

int AssetsManager::ReadScript(Stream *stream, std::unordered_map<std::string, GameObject *> &tempList) {
    std::size_t gnsize = 0;

    if (!stream->read(reinterpret_cast<char *>(&gnsize), sizeof(std::size_t))) {
        return 1;
    }

    std::string gname(gnsize, '\0');
    if (!stream->read(gname.data(), gnsize * sizeof(char))) {
        return 2;
    }

    auto go = tempList[gname];

    std::size_t ccount = 0;
    if (!stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 3;
    }


    std::size_t nsize = 0;
    for (std::size_t i=0;i<ccount;i++) {
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t))) {
            return 4;
        }

        std::string cname(nsize, '\0');
        if (!stream->read(cname.data(), nsize * sizeof(char))) {
            return 5;
        }

        std::size_t stream_size = 0;
        if (!stream->readS(&stream_size, 1)) {
            return 6;
        }
        
        auto nextPosition = stream->tellg() + static_cast<std::streampos>(stream_size);

        Event::Add(&AssetsManager::SetupScript, go, stream->getPath(), stream->tellg(), cname);

        stream->seekg(nextPosition, std::ios_base::beg);
    }


    if (!stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 7;
    }

    int result =0;
    for (std::size_t i=0;i<ccount;i++) {
        result = ReadScript(stream, tempList);
        if (result != 0) {
            break;
        }
    }

    return result != 0 ? 8 + result : 0;
}

void AssetsManager::CreateGameObject(std::string name, std::string tag, bool enable, bool active, Transform *parent, Transform *&out, std::condition_variable &cv, std::mutex &mutex, GameObject **gameOut) {
    auto gameObject = GameObject::Create(name, tag);
    if (!gameObject) {
        return;
    }
    
    if (!(*gameOut)) {
        *gameOut = gameObject;
    }

    gameObject->setEnable(active);
    gameObject->setActive(active);

    auto transform = gameObject->getTransform();
    if (parent) {
        transform->setParent(parent);
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        out = transform;
    }
    cv.notify_one();
}

int AssetsManager::ReadGameObject(Stream *stream, Transform *parent, GameObject **out, std::unordered_map<std::string, GameObject *> &tempList) {
    std::size_t nsize = 0;

    // write-name
    if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t))) {
        return 1;
    }

    std::string name(nsize, '\0');
    if (!stream->read(name.data(), nsize * sizeof(char))) {
        return 2;
    }

    // write-tag
    if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t))) {
        return 3;
    }

    std::string tag(nsize, '\0');
    if (!stream->read(tag.data(), nsize * sizeof(char))) {
        return 4;
    }

    auto bActive = false;
    auto bEnable = false;

    // write-status
    {
        if (!stream->read(reinterpret_cast<char *>(&bActive), sizeof(bool))) {
            return 5;
        }

        if (!stream->read(reinterpret_cast<char *>(&bEnable), sizeof(bool))) {
            return 6;
        }
    }

    Transform *transform = nullptr;
    std::condition_variable cv;

    Event::Add(&AssetsManager::CreateGameObject, name, tag, bEnable, bActive, parent, std::ref(transform), std::ref(cv), std::ref(s_gameObjectMutex), out);

    std::unique_lock<std::mutex> lock(s_gameObjectMutex);
    cv.wait(lock, [&transform] {
        return transform != nullptr;
    });
    if (transform)
        tempList.emplace(name, transform->getGameObject());
        
    lock.unlock();

    std::size_t children_size = 0;
    if (!stream->read(reinterpret_cast<char *>(&children_size), sizeof(std::size_t))) {
        return 7;
    }
    
    int result = 0;
    for (int i=0;i<children_size;i++) {
        result = ReadGameObject(stream, transform, out, tempList);
        if (result != 0) {
            break;
        }
    }
    transform = nullptr;

    return result != 0 ? (7 + result) : 0;
}

void AssetsManager::ProcessBinary() {
    std::unique_lock<std::mutex> lock(s_gameObjectMutex);
    while (!s_gameObjectFromLoader.empty()) {
        auto handle = s_gameObjectFromLoader.front();
        s_gameObjectFromLoader.pop_front();
        lock.unlock();

        doInBackgroundGameObject(handle);

        lock.lock();
    }
}

int AssetsManager::ReadTexture(Stream *stream, std::unordered_map<std::string, GameObject *> &tempList) {
    std::size_t nsize = 0;
    if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t))) {
        return 1;
    }

    std::string goname(nsize, '\0');
    if (!stream->read(goname.data(), nsize * sizeof(char))) {
        return 2;
    }

    std::size_t tcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&tcount), sizeof(std::size_t))) {
        return 3;
    }

    for (std::size_t i=0;i<tcount;i++) {
        std::size_t psize = 0;
        if (!stream->read(reinterpret_cast<char *>(&psize), sizeof(std::size_t))) {
            return 4;
        }

        std::string path(psize, '\0');
        if (!stream->read(path.data(), psize * sizeof(char))) {
            return 5;
        }

        AssetsManager::LoadTexture(path);
    }

    std::size_t ccount = 0;
    if (!stream->read(reinterpret_cast<char *>(&ccount), sizeof(std::size_t))) {
        return 6;
    }

    int result = 0;
    for (std::size_t i=0;i<ccount;i++) {
        result = ReadTexture(stream, tempList);
        if (result != 0) {
            break;
        }
    }
    return result;
}

void AssetsManager::SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func) {
    if (!File::Exists(path)) {
        func(nullptr);
        return Console::LogError("[ AssetsManager::GameObject ] File not found: " + path);
    }

    {
        std::lock_guard<std::mutex> lock(s_gameObjectMutex);
        s_gameObjectFromLoader.emplace_back(func, path);
    }

    Threadpool::AddTask(&AssetsManager::ProcessBinary);
}

// TODO: aicionar lista de callbacks a serem chamadas apos shader serem adicionado a thread
void AssetsManager::SetShaderList(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func) {
    if (!File::Exists(frag)) {
        return Console::LogError("[ AssetsManager-Shader ] File do not exists: " + frag);
    }

    if (!File::Exists(vert)) {
        return Console::LogError("[ AssetsManager-Shader ] File do not exists: " + vert);
    }

    auto it = s_shaderList.find(name);
    if (it != s_shaderList.end()) {
        if (func)
            func(&(*it->second));
        return;
    }

    {
        std::lock_guard<std::mutex> lock(s_shaderMutex);
        auto it = std::find_if(s_shaderFromLoader.begin(), s_shaderFromLoader.end(), [name](auto &handle) {
            return handle.m_name == name;
        });
        
        if (it != s_shaderFromLoader.end()) {
            it->add(func);
            return;
        }
        s_shaderFromLoader.emplace_back(func, name, frag, vert);
    }

    Threadpool::AddTask(&AssetsManager::ProcessShader);
}

void AssetsManager::SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func) {
    if (!File::Exists(path)) {
        return Console::LogError("[ AssetsManager::Texture ] File not found: " + path);
    }

    {
        std::lock_guard<std::mutex> lock(m_mutexTexture);
        auto map = textureMap.find(path);
        if (map != textureMap.end()) {
            if (func) {
                func(&(*map->second));
            }
            return;
        }

        auto it = m_mapHandleTexture.find(path);
        if (it != m_mapHandleTexture.end()) {
            it->second->add(func);
            return;
        }
        auto &handle = m_textureHandle.emplace_back(func, path);
        m_mapHandleTexture.emplace(path, &handle);
    }

    Threadpool::AddTask(&AssetsManager::ProcessTexture);
}

std::unordered_map<std::string, std::unique_ptr<grr::gShader>> &AssetsManager::GetShaderList() {
    return s_shaderList;
}

grm::size AssetsManager::GetTextureCount() {
    return textureMap.size();
}

grm::size AssetsManager::GetShaderCount() {
    return s_shaderList.size();
}

grm::size AssetsManager::GetMeshCount() {
    auto count = 0;
    {
        std::lock_guard<std::mutex> lock(m_meshMutex);
        count = s_meshList.size();
    }
    return count;
}

std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> &AssetsManager::GetTextureMap() {
    return textureMap;
}

TextureHandle::TextureHandle(std::function<void(grr::gTexture *)> function, std::string path) : m_path(path) {
    if (!function) {
        return;
    }
    m_functions.push_back(function);
}

void TextureHandle::invoke(grr::gTexture *texture) {
    for (auto it = m_functions.begin(); it != m_functions.end();) {
        if (*it) {
            (*it)(texture);
        }
        it = m_functions.erase(it);
    }
}

void TextureHandle::add(std::function<void(grr::gTexture *)> function) {
    m_functions.push_back(function);
}

ShaderHandle::ShaderHandle(std::function<void(grr::gShader *)> function, std::string name, std::string frag, std::string vert) : m_name(name), m_frag(frag), m_vert(vert) {
    if (!function) {
        return;
    }
    m_functions.push_back(function);
}

void ShaderHandle::invoke(grr::gShader *shader) {
    for (auto it = m_functions.begin(); it != m_functions.end();) {
        if (*it) {
            (*it)(shader);
        }
        it = m_functions.erase(it);
    }
}

void ShaderHandle::add(std::function<void(grr::gShader *)> function) {
    m_functions.push_back(function);
}

Material *AssetsManager::FindMaterial(const std::string &key) {
    std::lock_guard<std::mutex> lock(materialsMutex);
    auto it = materialsMap.find(key);
    if (it == materialsMap.end()) {
        return nullptr;
    }
    return &(*it->second);
}

std::string AssetsManager::GetRefMaterialToName(Material *material) {
    std::lock_guard<std::mutex> lock(materialsMutex);
    auto it = materialRefToName.find(material);
    if (it == materialRefToName.end()) {
        return "";
    }
    return it->second;
}

std::unordered_map<std::string, std::unique_ptr<Material>> &AssetsManager::GetMaterialMap() {
    return materialsMap;
}
