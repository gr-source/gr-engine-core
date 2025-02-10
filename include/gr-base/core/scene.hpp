#pragma once

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/default.hpp"

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

    static void DestroyScript();

    static void CheckRequestReinstanceScript();

    static void CleanTemp();

    static std::unordered_map<std::string, std::unique_ptr<Scene>> &GetScenes();

    static void CleanCurrentScene();

    static void ReloadObjects(std::function<void(Scene *)> func);

    static void InitializeEngineScript();

    static void InitializeAllScript();

    static void Remove(GameObject *gameObject);

    static void Destoy(GameObject *gameObject);

    static void Init(GameObject *gameObject);

    static Scene *AddScene(const std::string &name, std::unique_ptr<Scene> scene);

    Scene(const std::string &name);
    
    ~Scene();

    void setName(const std::string &name);

    std::string getName() const;
    
    void save(const std::string &path);

    GameObject *CreateGameObject(const std::string &name, const std::string &tag = "untagged");

    std::vector<GameObject *> &getObjectList();

    void insert(GameObject *gameObject);

    void Clear();

    GameObject *getCameraObject() const;

    void setCamera(GameObject *go);

    Camera *getCamera() const;

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

    GameObject *m_goCamera;

    Camera *m_camera;

    bool m_enable;

    static void InitializeInternalScript(Transform *transorm);

    static GameObject *CreateEngineCamera();

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    int writeScene(Stream *stream);

    int writeMetadata(Stream *stream);

    int writeTexture(Stream *stream);

    int writeMaterial(Stream *stream);

    int writeMesh(Stream *stream);

    int writeObject(Stream *stream);

    int writeGameObject(Stream *stream, GameObject *gameObject);

    int writeScript(Stream *stream);

    static void SetupScene(Scene *scene);

    static void SetupScript(GameObject *gameObject, std::string name, std::condition_variable &cv, Component *&component, int &result);

    static void ProcessScript(Component *component, std::string path, std::streampos position);

    bool remove(GameObject *gameObject);

    bool destroy(GameObject *gameObject);

    void for_each(std::function<void (GameObject *)> func);

    void for_each(Transform *transform, std::function<void (GameObject *)> func);

    void reinstanceScript(const std::string &name);

};

