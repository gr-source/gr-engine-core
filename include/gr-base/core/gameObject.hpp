#pragma once

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/default.hpp"

struct GameObject
{
    engine::ecs::dynamic::world* world;

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

    void* AddComponentByID(TypeID type);

    template <typename T>
    inline T* GetComponent() const
    {
        return world->GetComponent<T>(entity);
    }

    void* GetComponentByID(TypeID type) const;

    template <typename T>
    inline T* GetOrAddComponent()
    {
        return HasComponent<T>() ? GetComponent<T>() : AddComponent<T>();
    }

    inline void* GetOrAddComponentByID(TypeID type)
    {
        return HasComponentByID(type) ? GetComponentByID(type) : AddComponentByID(type);
    }

    template <typename... Components>
    bool HasComponent() const
    {
        return world->HasComponent<Components...>(entity);
    }

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

