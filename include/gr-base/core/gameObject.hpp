#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/typeRegistry.hpp"
#include "gr-base/ecs/default.hpp"

struct GameObject
{
    Scene* scene;

    Entity entity;

    bool IsAlive() const;

    void* AddComponent(TypeID typeID, const void* src = nullptr);

    inline void* AddComponent(const char* name)
    {
        return AddComponent(TypeRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* AddComponent(T temp = {})
    {
        return static_cast<T*>(AddComponent(TypeRegistry::GetTypeID<T>(), &temp));
    }

    void* GetComponent(TypeID typeID) const;

    inline void* GetComponent(const char* name) const
    {
        return GetComponent(TypeRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetComponent() const
    {
        return static_cast<T*>(GetComponent(TypeRegistry::GetTypeID<T>()));
    }

    void* GetOrAddComponent(TypeID typeID);

    inline void* GetOrAddComponent(const char* name)
    {
        return GetOrAddComponent(TypeRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetOrAddComponent()
    {
        return static_cast<T*>(GetOrAddComponent(TypeRegistry::GetTypeID<T>()));
    }

    bool HasComponent(Signature mask) const;

    template <typename... Components>
    bool HasComponent() const
    {
        return HasComponent(TypeRegistry::GetMask<Components...>());
    }

    void DelComponent(TypeID typeID);

    inline void DelComponent(const char* name)
    {
        DelComponent(TypeRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline void DelComponent()
    {
        DelComponent(TypeRegistry::GetTypeID<T>());
    }
};

