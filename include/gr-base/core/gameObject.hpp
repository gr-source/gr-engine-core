#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/type.hpp"

struct GameObject
{
    Scene* scene;

    Entity entity;

    void* AddComponent(TypeID typeID, const void* src = nullptr);

    inline void* AddComponent(const char* name)
    {
        return AddComponent(Type::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* AddComponent()
    {
        return static_cast<T*>(AddComponent(Type::GetTypeID<T>()));
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
};

