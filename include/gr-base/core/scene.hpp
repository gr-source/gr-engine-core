#pragma once

#include "../script/default.hpp"
#include "ecs/default.hpp"

#include "geometry/material.hpp"

#include <condition_variable>
#include <functional>
#include <list>

class Stream;

// TODO: Causando crash quando d aplya na cena e para o jogo, motivo esta nos objetos que nao foram reacceragdos das cenas
class Scene {
public:
    static void SetCurrent(Scene *scene);

    static Scene *GetCurrentScene();

    static bool HasScene();

    static void Use(Scene *scene);
    
    static void Initialize();

    static void Shutdown();

    static Scene *Create(const std::string &name);

    static void LoadScene(const std::string &path, std::function<void(Scene *)> func = nullptr);

    static void UpdateScript();

    static void DestroyScript();

    static void SetupEssential();

    static void CheckRequestReinstanceScript();

    static void CleanTemp();

    static std::unordered_map<std::string, std::unique_ptr<Scene>> &GetScenes();

    static void CleanCurrentScene();

    static void ReloadObjects(std::function<void(Scene *)> func);

    static void InitializeObject();

    static void Remove(GameObject *gameObject);

    static void Destoy(GameObject *gameObject);

    static void Init(GameObject *gameObject);

    Scene(const std::string &name);
    
    ~Scene();

    std::string getName() const;
    
    void save(const std::string &path);

    GameObject *CreateGameObject(const std::string &name, const std::string &tag = "untagged");

    std::vector<GameObject *> &getObjectList();

    void insert(GameObject *gameObject);

    void Clear();

private:
    static std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;

    static std::hash<std::string> hash;

    static std::condition_variable cv;

    static std::mutex m_mutex;

    static Scene *m_current;

    static bool bQuit;

    std::vector<std::tuple<std::string, std::string, Component *>> m_componentCache; 

    std::vector<GameObject *> m_objects;

    std::string m_name;

    std::string m_path;

    bool m_enable;

    enum LoadFlags : std::uint8_t {
        LoadFlags_Mesh    = 1 << 1,
        LoadFlags_Object  = 1 << 2,
        LoadFlags_Texture = 1 << 3,
        LoadFlags_Script  = 1 << 4,
    };

    static void Load(const std::string &path, std::uint8_t flags = LoadFlags_Mesh | LoadFlags_Object | LoadFlags_Texture | LoadFlags_Script, std::function<void(Scene *)> func = nullptr, Scene *scene = nullptr);

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    int writeScene(Stream *streaming);

    int writeTexture(Stream *streaming);

    int writeMesh(Stream *streaming);

    int writeObject(Stream *streaming);

    int writeScript(Stream *streaming);

    int writeGameObject(Stream *streaming, GameObject *gameObject);

    static int readObject(Stream *streaming, Scene *scene, std::unordered_map<std::string, GameObject *> &tempObject);

    static int readTexture(Stream *streaming);

    static int readMesh(Stream *streaming);

    static int readScript(Stream *streaming, std::unordered_map<std::string, GameObject *> &tempObject);

    static int readNode(Stream *streaming, Scene *scene, Transform *parent, std::unordered_map<std::string, GameObject *> &tempObject);

    static void SetupScene(Scene *scene);

    static void SetupScript(GameObject *gameObject, std::string name, std::condition_variable &cv, Component *&component, int &result);

    static void ProcessScript(Component *component, std::string path, std::streampos position);

    static void Initialize(GameObject *go);

    bool remove(GameObject *gameObject);

    bool destroy(GameObject *gameObject);

    void for_each(std::function<void (GameObject *)> func);

    void for_each(Transform *transform, std::function<void (GameObject *)> func);

    void reinstanceScript(const std::string &name);

};
