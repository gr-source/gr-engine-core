#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/type_id.hpp"
#include "gr-base/ecs/world.hpp"

#include "gr-base/spatial/default.hpp"

#include <unordered_set>
#include <string>

enum class StageType
{
    INPUT,
    SIMULATION,
    RENDER,
    COUNT
};

struct ChangeSet
{
    std::vector<Entity> entities;
    std::vector<int> frames;
};

class ISystem;

class Scene
{
public:
    engine::ecs::world* world;

    inline engine::ecs::world* GetWorldDynamic()
    {
        return world;
    }

    static Scene* GetCurrent();

    bool hierarchyDirty;

    Scene();
    ~Scene();

    inline Entity GetRoot() const
    {
        return m_root;
    }

    Entity create(const char* name);

    Entity create(Entity other);

    void destroy(Entity e);

    void select(Handle id);

    void deselect(Handle id);

    void deselect_all();

    bool has_object_selected() const;

    bool object_has_been_selected(Entity id) const;

    std::vector<Handle> get_object_selected() const;

    void SetName(const std::string& name);

    std::string GetName() const;

    inline BVHtree* GetBVHtree() const
    {
        return m_bvhtree;
    }

    size_t GetEntityCount();

    void Unlink(Entity child);

    void SetParent(Entity parent, Entity child);

    inline void SetCamera(Entity entity)
    {
        m_camera = entity;
    }

    inline Entity GetCamera() const
    {
        return m_camera;
    }

    void BeginFrame();

    void EndFrame();

    void Update();

    template <typename T>
    inline void MarkChanged(Entity entity)
    {
        MarkChanged(GetComponentID<T>::id, entity);
    }

    void MarkChanged(TypeID typeID, Entity entity);

    template <typename T>
    bool Changed(std::vector<Entity>& entities) const
    {
        return Changed(GetComponentID<T>::id, entities);
    }

    bool Changed(TypeID typeID, std::vector<Entity>& entities) const;

    void AddSystemByID(TypeID systemID);

    template <typename T>
    void AddSystem();

    void DelSystemByID(TypeID systemID);

    template <typename T>
    void DelSystem();

    ISystem* GetSystemByID(TypeID id) const;

    template <typename T>
    T* GetSystem() const;

    std::vector<TypeID> GetSystemIDs() const;

private:
    std::unordered_set<Handle> u_object_has_been_selected;

    std::vector<Entity> v_object_selected_list;

    int readIndex = 0;
    int writeIndex = 1;

    std::vector<ChangeSet> changes[2];

    BVHtree* m_bvhtree;

    std::string m_name;

    Entity m_root;

    Entity m_camera;

    int currentFrame = 0;

    // systems
    std::vector<TypeID> m_systemIDs;

    std::vector<ISystem*> m_typeIDToSystem;

    std::vector<ISystem*> m_systems;

    ISystem* createSystem(const SystemInfo* info);

    void sortSystems();
};


template <typename T>
inline void Scene::AddSystem()
{
    auto systemID = GetSystemID<T>::id;
    if (systemID != INVALID_TYPE)
        AddSystemByID(systemID);
}

template <typename T>
inline void Scene::DelSystem()
{
    auto systemID = GetSystemID<T>::id;
    if (systemID != INVALID_TYPE)
        DelSystemByID(systemID);
}

template <typename T>
inline T* Scene::GetSystem() const
{
    auto systemID = GetSystemID<T>::id;
    if (systemID != INVALID_TYPE)
        return static_cast<T*>(GetSystemByID(systemID));
    return nullptr;
}



