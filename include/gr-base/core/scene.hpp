#pragma once

#include "gr-base/script/default.hpp"

#include "default.hpp"

#include <unordered_set>
#include <functional>
#include <string>

class Stream;

class Scene {
public:
    // static void CheckRequestReinstanceScript();

    Scene();
    ~Scene();

    void init();

    void play();

    void stop();

    void create_main_camra();

    bool is_running() const;

    std::vector<GameObjectID> getObjectList() const;

    void insert(GameObjectID gameObjectID);

    void remove(GameObjectID gameObjectID);

    void Clear();

    GameObjectID getCameraObject() const;

    void setCamera(GameObjectID id);

    Camera *getCamera() const;

    void select(GameObjectID id);

    void deselect(GameObjectID id);

    void deselect_all();

    bool has_object_selected() const;

    bool object_has_been_selected(GameObjectID id) const;

    std::vector<GameObjectID> get_object_selected() const;

    bool destroy(GameObjectID gameObjectID);

    static void Play(GameObject *go);

    static void Stop(GameObject *go);
private:
    std::vector<std::tuple<std::string, std::string, Component *>> m_componentCache; 

    // std::vector<GameObject *> m_tempInitObjects;

    std::vector<GameObjectID> m_objects;

    std::unordered_set<GameObjectID> u_object_has_been_selected;

    std::vector<GameObjectID> v_object_selected_list;

    GameObjectID m_goCamera;

    Camera *m_camera;

    bool m_enable;

    bool m_isrunning;

    static GameObjectID CreateEngineCamera();

    void setEnable(bool value);

    void setDisableEntity(Transform *transform, bool value);

    void for_each(std::function<void (GameObject *)> func);

    void for_each(Transform *transform, std::function<void (GameObject *)> func);

    void reinstanceScript(const std::string &name);

};

