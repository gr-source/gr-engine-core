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
    static void DestroyScript();

    static void CheckRequestReinstanceScript();

    static void InitializeEngineScript();

    static void InitializeAllScript();

    static void Remove(GameObject *gameObject);

    Scene();
    ~Scene();

    void init();

    void create_main_camra();

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

    bool destroy(GameObject *gameObject);

private:
    static std::hash<std::string> hash;

    std::vector<std::tuple<std::string, std::string, Component *>> m_componentCache; 

    std::vector<GameObject *> m_tempInitObjects;

    std::vector<GameObject *> m_objects;

    std::unordered_set<GameObject *> u_object_has_been_selected;

    std::vector<GameObject *> v_object_selected_list;

    GameObject *m_goCamera;

    Camera *m_camera;

    bool m_enable;

    static void InitializeInternalScript(Transform *transorm);

    static GameObject *CreateEngineCamera();

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    bool remove(GameObject *gameObject);

    void for_each(std::function<void (GameObject *)> func);

    void for_each(Transform *transform, std::function<void (GameObject *)> func);

    void reinstanceScript(const std::string &name);

};

