#pragma once

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/default.hpp"

struct GameObject
{
    engine::ecs::world* world;

    Scene* scene;

    Entity entity;

    inline GameObject(Scene* scene, Entity entity) :
        scene(scene), entity(entity), world(scene->GetWorldDynamic())
    {}

    inline bool IsAlive() const
    {
        return world != nullptr && world->IsAlive(entity);
    }

    std::string GetName() const;

    template <typename T, typename... Args>
    inline T* AddComponent(Args&&... args)
    {
        return world->AddComponent<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    inline T* GetComponent() const
    {
        return world->GetComponent<T>(entity);
    }

    template <typename T>
    inline T* GetOrAddComponent()
    {
        return HasComponent<T>() ? GetComponent<T>() : AddComponent<T>();
    }

    template <typename... Components>
    bool HasComponent() const
    {
        return world->HasComponent<Components...>(entity);
    }

    inline void* GetOrAddComponentByID(TypeID type)
    {
        return HasComponentByID(type) ? GetComponentByID(type) : AddComponentByID(type);
    }

    void* AddComponentByName(const std::string& name);

    void* AddComponentByID(TypeID type);

    void* GetComponentByName(const std::string& name) const;

    void* GetComponentByID(TypeID type) const;

    bool HasComponentByName(const std::string& name) const;

    bool HasComponentByID(TypeID type) const;

    std::vector<TypeID> GetAllComponentIDs() const;

    /*
    template <typename T>
    inline void DelComponent()
    {
        DelComponent(ComponentRegistry::GetTypeID<T>());
    }
    */
};

