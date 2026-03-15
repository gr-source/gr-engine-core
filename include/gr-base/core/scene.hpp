#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

#include "default.hpp"

#include <unordered_set>
#include <functional>
#include <string>

class Scene
{
public:
    static Scene* GetCurrent();

    bool bRunning;

    Scene();
    ~Scene();

    std::vector<GameObjectHandle> getObjectList() const;

    void insert(GameObjectHandle gameObjectID);

    void remove(GameObjectHandle gameObjectID);

    void Clear();

    void SetCamera(ComponentHandle id);

    Camera *getCamera() const;

    void select(GameObjectHandle id);

    void deselect(GameObjectHandle id);

    void deselect_all();

    bool has_object_selected() const;

    bool object_has_been_selected(GameObjectHandle id) const;

    std::vector<GameObjectHandle> get_object_selected() const;

    bool destroy(GameObjectHandle gameObjectID);

    void AddLight(ComponentHandle id);

    void RemoveLight(ComponentHandle id);

    std::vector<ComponentHandle> GetLights() const;

    void AddCamera(ComponentHandle id);

    void RemoveCamera(ComponentHandle id);

    std::vector<ComponentHandle> GetCameras() const;

    void SetName(const std::string& name);

    std::string GetName() const;

    void AddMesh(MeshID meshID);

    void Update();

private:
    std::unordered_set<std::string> m_chunksLoaded;

    std::vector<ComponentHandle> m_cameras;

    std::vector<ComponentHandle> m_lights;

    std::vector<GameObjectHandle> m_objects;

    std::unordered_set<GameObjectHandle> u_object_has_been_selected;

    std::vector<GameObjectHandle> v_object_selected_list;

    std::vector<MeshID> m_meshes;

    std::string m_name;

    ComponentHandle m_cameraID;

    void loadChunk(std::string name);

    static GameObjectHandle CreateEngineCamera();
};



