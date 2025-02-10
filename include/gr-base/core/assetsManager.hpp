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

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/task/textureTask.hpp"
#include "gr-base/core/task/sceneTask.hpp"

class GameObjectHandle {
public:
    GameObjectHandle(std::function<void(GameObject *)> &function, const std::string &path) : m_function(function), m_gameObject(nullptr), m_path(path) {}

    std::function<void(GameObject *)> m_function;

    GameObject *m_gameObject;

    std::string m_path;
};

class ShaderHandle {
public:
    ShaderHandle(std::function<void(grr::gShader *)> function, std::string path);

    void invoke(grr::gShader *shader);

    void add(std::function<void(grr::gShader *)> function);

    std::string m_path;

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

enum ShaderType {
    ShaderType_None     = 1 << 0,
    ShaderType_Fragment = 1 << 1,
    ShaderType_Vertex   = 1 << 2,
};

enum UniformType {
    UniformType_object    = 1 << 0,
    UniformType_float     = 1 << 1,
    UniformType_int       = 1 << 2,
    UniformType_bool      = 1 << 3,
    UniformType_mat3      = 1 << 4,
    UniformType_mat4      = 1 << 5,
    UniformType_vec2      = 1 << 6,
    UniformType_vec3      = 1 << 7,
    UniformType_vec4      = 1 << 8,
    UniformType_sampler2D = 1 << 9,
    UniformType_samplerCube = 1 << 10,
};

struct Uniform {
    std::string name;
    std::string type_name;
    UniformType type;
};

struct ShaderData {
    std::string source;

    std::vector<Uniform> uniformList;
};

struct UniformStackInfo {
    std::string name;
    std::string object;
    int count;
};

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

    template <typename ...Args>
    static void LoadGameObject(const std::string &path, Args &&...args);

    template <typename ...Args>
    static void LoadScene(const std::string &path, Args &&...args);

    static Animation *LoadAnimation(const std::string &path);

    static grr::gShader *FindShader(const std::string &name);

    static void UpdateShader(grr::gShader *shader, const std::string &path);

    // ********** Texture ********** //
    static grr::gTexture *CreateTexture(const std::string &name);

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

    static void SetShaderList(const std::string &path, std::function<void(grr::gShader *)> func);

    static void SetTextureList(const std::string &path, std::function<void(grr::gTexture *)> func);

    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> &GetShaderList();

    template <typename ...Args>
    static Material *CreateMaterial(const std::string &key, Args &&...args);

    static Material *FindMaterial(const std::string &key);

    static std::string GetRefMaterialToName(Material *material);

    static std::unordered_map<std::string, std::unique_ptr<Material>> &GetMaterialMap();

    static void RemoveTask(const std::string &name);

private:
    /***************** Shader *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gShader>> mShaderList;
    static std::unordered_map<grr::gShader *, std::string> mShaderPathRef;
    static std::list<ShaderHandle> mShaderFromLoad;

    // Update Shader
    static std::unordered_map<grr::gShader *, std::string> mShaderFromUpdate;
    static std::list<grr::gShader *> mTaskList;
    static std::mutex mShaderMutex;

    /***************** Texture *****************/
    static std::unordered_map<std::string, std::unique_ptr<grr::gTexture>> mTextureMap;
    static std::unordered_map<grr::gTexture *, std::string> mTextureRefToName;

    // multithread
    static std::unordered_map<std::string, std::unique_ptr<TextureTask>> mTextureTaskMap;
    static std::mutex mTextureMutex;

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

    /***************** Scene *****************/
    static std::unordered_map<std::string, std::unique_ptr<SceneTask>> mSceneTaskMap;
    static std::unordered_map<std::string, std::unique_ptr<Scene>> mSceneMap;
    static std::mutex mSceneMutex;

    static void SetUpdateShader(grr::gShader *shader, ShaderData frag, ShaderData vert, std::string name);

    static void TaskUpdateShader();

    static void SetupShader(ShaderHandle handle, ShaderData frag, ShaderData vert);
    
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

    static std::map<ShaderType, ShaderData> ReadShaderOfType(const std::string &path);

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

template <typename... Args>
inline void AssetsManager::LoadShader(const std::string &path, Args &&...args) {
    std::function<void(grr::gShader *)> func = nullptr;
    if constexpr (sizeof...(Args) > 0) {
        func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
    }
    
    SetShaderList(path, func);
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





