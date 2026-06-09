#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/default.hpp"

struct GameObject
{
    Scene* scene;

    Entity entity;

    bool IsAlive() const;

    void* AddComponent(TypeID typeID, const void* src = nullptr);

    inline void* AddComponent(const char* name)
    {
        return AddComponent(ComponentRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* AddComponent(T temp = {})
    {
        return static_cast<T*>(AddComponent(ComponentRegistry::GetTypeID<T>(), &temp));
    }

    void* GetComponent(TypeID typeID) const;

    inline void* GetComponent(const char* name) const
    {
        return GetComponent(ComponentRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetComponent() const
    {
        return static_cast<T*>(GetComponent(ComponentRegistry::GetTypeID<T>()));
    }

    void* GetOrAddComponent(TypeID typeID);

    inline void* GetOrAddComponent(const char* name)
    {
        return GetOrAddComponent(ComponentRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetOrAddComponent()
    {
        return static_cast<T*>(GetOrAddComponent(ComponentRegistry::GetTypeID<T>()));
    }

    bool HasComponent(Signature mask) const;

    template <typename... Components>
    bool HasComponent() const
    {
        return HasComponent(ComponentRegistry::GetMask<Components...>());
    }

    void DelComponent(TypeID typeID);

    inline void DelComponent(const char* name)
    {
        DelComponent(ComponentRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline void DelComponent()
    {
        DelComponent(ComponentRegistry::GetTypeID<T>());
    }
};

