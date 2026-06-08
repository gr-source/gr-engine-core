#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/type.hpp"

struct GameObject
{
    Scene* scene;

    Entity entity;

    bool IsAlive() const;

    void* AddComponent(TypeID typeID, const void* src = nullptr);

    inline void* AddComponent(const char* name)
    {
        return AddComponent(Type::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* AddComponent(T temp = {})
    {
        return static_cast<T*>(AddComponent(Type::GetTypeID<T>(), &temp));
    }

    void* GetComponent(TypeID typeID) const;

    inline void* GetComponent(const char* name) const
    {
        return GetComponent(Type::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetComponent() const
    {
        return static_cast<T*>(GetComponent(Type::GetTypeID<T>()));
    }

    void* GetOrAddComponent(TypeID typeID);

    inline void* GetOrAddComponent(const char* name)
    {
        return GetOrAddComponent(Type::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetOrAddComponent()
    {
        return static_cast<T*>(GetOrAddComponent(Type::GetTypeID<T>()));
    }

    bool HasComponent(Signature mask) const;

    template <typename... Components>
    bool HasComponent() const
    {
        return HasComponent(Type::GetMask<Components...>());
    }

    void DelComponent(TypeID typeID);

    inline void DelComponent(const char* name)
    {
        DelComponent(Type::GetTypeIDByName(name));
    }

    template <typename T>
    inline void DelComponent()
    {
        DelComponent(Type::GetTypeID<T>());
    }
};

