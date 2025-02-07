#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <list>
#include <map>

#include <gr-render/gRender.h>

#include "gr-base/core/animation/animation.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/filmbox/default.hpp"

#include "geometry/material.hpp"

#include "gr-base/script/default.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/core/geometry/mesh.hpp"

class GameObjectHandle {
public:
    GameObjectHandle(std::function<void(GameObject *)> &function, const std::string &path) : m_function(function), m_gameObject(nullptr), m_path(path) {}

    std::function<void(GameObject *)> m_function;

    GameObject *m_gameObject;

    std::string m_path;
};

class ShaderHandle {
public:
    ShaderHandle(std::function<void(grr::gShader *)> function, std::string name, std::string frag, std::string vert);

    void invoke(grr::gShader *shader);

    void add(std::function<void(grr::gShader *)> function);

    std::string m_name;
    
    std::string m_frag;

    std::string m_vert;

private:
    std::vector<std::function<void(grr::gShader *)>> m_functions;
};

class TextureHandle {
public:
    std::string m_path;

    TextureHandle(std::function<void(grr::gTexture *)> function, std::string path);

    void invoke(grr::gTexture *texture);

    void add(std::function<void(grr::gTexture *)> function);

private:
    std::vector<std::function<void(grr::gTexture *)>> m_functions;
};

class AssetsManager {
public:
    static void Initialize();

    static void Shutdown();

    template <typename ...Args>
    static void LoadTexture(const std::string &path, Args &&...args);

    static std::string GetRefTextureName(grr::gTexture *texture);

    template <typename ...Args>
    static void LoadShader(const std::string &name, const std::string &frag, const std::string &vert, Args &&...args);

    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    static Animation *LoadAnimation(const std::string &path);

    static grr::gShader *FindShader(const std::string &name);

    // ********** Mesh ********** //
    template <typename ...Args>
    static Mesh *CreateMesh(const std::string &key, Args &&...args);

    static Mesh *FindMesh(const std::string &name);

    static void DeleteMesh(Mesh *target);

    static std::unordered_map<std::string, std::unique_ptr<Mesh>> &GetMeshListMap();

    static grm::u32 m_processCount;

    static grm::size GetTextureCount();

    static grm::size GetShaderCount();
    
    static grm::size GetMeshCount();

    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> &GetTextureMap();

    static void SetGameObjectList(const std::string &path, std::function<void(GameObject *)> func);

    static void SetShaderList(const std::string &name, const std::string &frag, const std::string &vert, std::function<void(grr::gShader *)> func);

    static void SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func);

    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> &GetShaderList();

    template <typename ...Args>
    static Material *CreateMaterial(const std::string &key, Args &&...args);

    static Material *FindMaterial(const std::string &key);

    static std::string GetRefMaterialToName(Material *material);

    static std::unordered_map<std::string, std::unique_ptr<Material>> &GetMaterialMap();

private:
    /***************** Shader *****************/
    static std::unordered_map<grr::gShader *, std::tuple<std::string, std::string>> mShaderPathRef;
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> mShaderList;
    static std::list<ShaderHandle> mShaderFromLoad;
    static std::mutex mShaderMutex;

    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> textureMap;
    static std::unordered_map<grr::gTexture *, std::string> textureRefToName;


    // multithread
    static std::unordered_map<std::string, TextureHandle *> m_mapHandleTexture;
    static std::list<TextureHandle> m_textureHandle;
    static std::mutex m_mutexTexture;

    /***************** Animation *****************/
    static std::unordered_map<std::string, std::unique_ptr<Animation>> s_animationList;
    static std::list<std::tuple<std::string, Animation *>> s_animationFromLoader;
    static std::mutex s_animatioMutex;

    /***************** GameObject *****************/
    static std::list<GameObjectHandle> s_gameObjectFromLoader;
    static std::mutex s_gameObjectMutex;

    /***************** Mesh *****************/
    static std::unordered_map<std::string, std::unique_ptr<Mesh>> s_meshList;
    static std::mutex m_meshMutex;
    
    /***************** Material *****************/
    static std::unordered_map<std::string, std::unique_ptr<Material>> materialsMap;
    static std::unordered_map<Material *, std::string> materialRefToName;
    static std::mutex materialsMutex;

    static void SetupTexture(TextureHandle handle, int width, int height, int ch, void *pixels);

    static void ProcessTexture();

    static void SetupShader(ShaderHandle handle, const std::string &frag, const std::string &vert);
    
    static void ProcessShader();

    static void SetupAnimation(const std::string &path, FbxAnimation *temp, Animation *target);

    static void doAnimationInBackground(const std::string &path, Animation *target);

    static void ProcessAnimation();

    static void doInBackgroundGameObject(GameObjectHandle handle);

    static void ReadData(Stream *stream, GameObjectHandle handle);

    static int ReadMaterial(Stream *stream, std::unordered_map<std::string, GameObject *> &gameObjectMap);

    static void CreateComponent(GameObject *gameObject, std::string componentName);

    static void SetupScript(GameObject *gameObject, std::string path, std::streampos position, std::string componentName);

    static int ReadScript(Stream *stream, std::unordered_map<std::string, GameObject *> &tempList);

    static void CreateGameObject(std::string name, std::string tag, bool enable, bool active, Transform *parent, Transform *&out, std::condition_variable &cv, std::mutex &mutex, GameObject **gameOut);

    static int ReadGameObject(Stream *stream, Transform *parent, GameObject **out, std::unordered_map<std::string, GameObject *> &tempList);

    static void ProcessBinary();

    static int ReadTexture(Stream *stream, std::unordered_map<std::string, GameObject *> &tempList);
};

template <typename... Args>
inline void AssetsManager::LoadGameObject(const std::string &path, Args &&...args) {
    std::function<void(GameObject *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    SetGameObjectList(path, func);
}

template <typename ...Args>
inline void AssetsManager::LoadTexture(const std::string &path, Args &&...args) {
    std::function<void(grr::gTexture *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }

    SetTextureList(path, func);
}

template <typename... Args>
inline void AssetsManager::LoadShader(const std::string &name, const std::string &frag, const std::string &vert, Args &&...args) {
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    
    SetShaderList(name, frag, vert, func);
}

template <typename... Args>
inline Mesh *AssetsManager::CreateMesh(const std::string &key, Args &&...args) {
    std::lock_guard<std::mutex> lock(m_meshMutex);
    auto it = s_meshList.find(key);
    if (it != s_meshList.end()) {
        return &(*it->second);
    }
    s_meshList.emplace(key, std::make_unique<Mesh>(std::forward<Args>(args)...));

    return &(*s_meshList[key]);
}

// ***** Material ***** //
template <typename... Args>
inline Material *AssetsManager::CreateMaterial(const std::string &key, Args && ...args) {
    std::lock_guard<std::mutex> lock(materialsMutex);
    auto it = materialsMap.find(key);
    if (it != materialsMap.end()) {
        return &(*it->second);
    }
    materialsMap[key] = std::make_unique<Material>(std::forward<Args>(args)...);

    materialRefToName[&(*materialsMap[key])] = key;

    return &(*materialsMap[key]);
}





