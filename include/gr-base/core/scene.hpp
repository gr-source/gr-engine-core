#pragma once

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "geometry/material.hpp"

#include <condition_variable>
#include <functional>
#include <list>
#include <map>
#include <unordered_set>

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

    static std::unordered_map<std::string, std::unique_ptr<Scene>> &GetScenes();

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

    std::vector<GameObject *> getObjectList() const;

    void insert(GameObject *gameObject);

    void Clear();

    GameObject *getCameraObject() const;

    void setCamera(GameObject *go);

    Camera *getCamera() const;

    void select(GameObject *go);

    void deselect(GameObject *go);

    void deselect_all();

    bool has_object_selected() const;

    bool object_has_been_selected(GameObject *go) const;

    std::vector<GameObject *> get_object_selected() const;

private:
    static std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;

    static std::hash<std::string> hash;

    static std::condition_variable cv;

    static std::mutex m_mutex;

    static Scene *m_current;

    static bool bQuit;

    std::vector<std::tuple<std::string, std::string, Component *>> m_componentCache; 

    std::vector<GameObject *> m_tempInitObjects;

    std::vector<GameObject *> m_objects;

    std::unordered_set<GameObject *> u_object_has_been_selected;

    std::vector<GameObject *> v_object_selected_list;

    std::string m_name;

    GameObject *m_goCamera;

    Camera *m_camera;

    bool m_enable;

    static void InitializeInternalScript(Transform *transorm);

    static GameObject *CreateEngineCamera();

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    static void SetupScene(Scene *scene);

    bool remove(GameObject *gameObject);

    bool destroy(GameObject *gameObject);

    void for_each(std::function<void (GameObject *)> func);

    void for_each(Transform *transform, std::function<void (GameObject *)> func);

    void reinstanceScript(const std::string &name);

};

