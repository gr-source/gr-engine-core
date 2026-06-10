#pragma once

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/default.hpp"

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/systemRegistry.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/spatial/default.hpp"

#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

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
    std::vector<bool> exists;
};

class Scene
{
public:
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

    inline GameObject Wrap(Entity e)
    {
        return {this, e};
    }

    void destroy(Entity e);

    void select(Handle id);

    void deselect(Handle id);

    void deselect_all();

    bool has_object_selected() const;

    bool object_has_been_selected(Entity id) const;

    std::vector<Handle> get_object_selected() const;

    void SetName(const std::string& name);

    std::string GetName() const;

    inline World* GetWorld() const
    {
        return m_world;
    }

    inline BVHtree* GetBVHtree() const
    {
        return m_bvhtree;
    }

    size_t GetEntityCount() const;

    void MarkChildrenDirty(Entity entity);

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

    std::string GetEntityName(Entity entity);

    void Update();

    template <typename T>
    inline void MarkChanged(Entity e)
    {
        MarkChanged(ComponentRegistry::GetTypeID<T>(), e);
    }

    void MarkChanged(TypeID type, Entity e);

    template <typename T>
    std::vector<Entity> Changed()
    {
        return Changed(ComponentRegistry::GetTypeID<T>());
    }

    std::vector<Entity> Changed(TypeID type);

    void AddSystemByID(TypeID systemID);

    template <typename T>
    void AddSystem();

    void DelSystemByID(TypeID systemID);

    template <typename T>
    void DelSystem();

    inline const std::vector<TypeID>& GetSystemIDs() const
    {
        return m_systemIDs;
    }

    inline ISystem* GetSystem(TypeID id) const
    {
        if (id >= m_typeIDToSystem.size())
            return nullptr;
        return m_typeIDToSystem[id];
    }

private:
    std::unordered_set<Handle> u_object_has_been_selected;

    std::vector<Entity> v_object_selected_list;

    std::vector<ChangeSet> changes;

    BVHtree* m_bvhtree;

    World* m_world;

    std::string m_name;

    Entity m_root;

    Entity m_camera;

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
    auto systemID = SystemRegistry::GetTypeID<T>();
    if (systemID != INVALID_TYPE)
        AddSystemByID(systemID);
}

template <typename T>
inline void Scene::DelSystem()
{
    auto systemID = SystemRegistry::GetTypeID<T>();
    if (systemID != INVALID_TYPE)
        DelSystemByID(systemID);
}



