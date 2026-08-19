#pragma once

#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/default.hpp"

struct GameObject
{
    Scene* scene;

    Entity entity;

    GameObject(Scene* scene, Entity entity);

    bool IsAlive() const;

    std::string GetName() const;

    template <typename T, typename... Args>
    inline T& AddComponent(Args&&... args)
    {
        return scene->world.AddComponent<T>(entity, std::forward<Args>(args)...);
    }


    template <typename T, typename... Args>
    inline T* TryAddComponent(Args&&... args)
    {
        return scene->world.TryAddComponent<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T>
    inline T& GetComponent() const
    {
        return scene->world.GetComponent<T>(entity);
    }

    template <typename T>
    inline T* TryGetComponent() const
    {
        return scene->world.TryGetComponent<T>(entity);
    }

    template <typename T>
    inline T& GetOrAddComponent()
    {
        return HasComponent<T>() ? GetComponent<T>() : AddComponent<T>();
    }

    template <typename... Components>
    bool HasComponent() const
    {
        return scene->world.HasComponent<Components...>(entity);
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


    explicit operator bool() const
    {
        return IsAlive();
    }
};

