#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/type_id.hpp"
#include "gr-base/ecs/world.hpp"

#include "gr-base/spatial/bvhtree.hpp"

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
    engine::ecs::world world;
    BVHtree bvhtree;

    static Scene* GetCurrent();

    Scene();
    ~Scene();

    Entity create(const char* name);

    Entity create(Entity other);

    void destroy(Entity entity);

    void SetName(const std::string& name);

    std::string GetName() const;

    size_t GetEntityCount();

    Entity GetRoot() const;

    void Unlink(Entity child);

    void SetParent(Entity parent, Entity child);

    void SetCamera(Entity entity);

    Entity GetCurrentCamera() const;

    void SetMainCamera(Entity entity);

    Entity GetMainCamera() const;

    bool CanSyncHierarchyUI() const;

    void GetRootEntities(std::vector<Entity>& entities) const;

    void BeginFrame();

    void EndFrame();

    void Update();

    void Render();

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
    int readIndex = 0;
    int writeIndex = 1;

    std::vector<ChangeSet> changes[2];

    std::string m_name;

    Entity m_root;

    Entity currentCamera = INVALID_ENTITY;

    Entity mainCamera = INVALID_ENTITY;

    int currentFrame = 0;

    bool hierarchySync[2] = { false, false };

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



