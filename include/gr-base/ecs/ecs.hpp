#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/type.hpp"

#include <unordered_map>
#include <memory>
#include <vector>

class ECS
{
public:
    std::unordered_map<Signature, std::shared_ptr<ArchetypeStorage>> m_signArcheTypeMap;

    ECS();

    Entity Create();

    void Destroy(Entity e);

    bool IsAlive(Entity e) const;

    std::vector<TypeID> intersection(ArchetypeStorage* src, ArchetypeStorage* dst);

    void MoveEntity(ArchetypeStorage* src, ArchetypeStorage* dst, Entity e);

    ArchetypeStorage* GetOrCreateArcheType(Signature mask, TypeID typeID);

    Signature GetMask(Entity e) const;

    void* AddComponent(Entity e, TypeID typeID, const void* src);

    inline void* AddComponent(Entity e, const char* name, const void* src)
    {
        return AddComponent(e, Type::GetTypeIDByName(name), src);
    }

    template <typename T, typename... Args>
    T* AddComponent(Entity e, Args&&... args)
    {
        T temp{std::forward<Args>(args)...};

        return static_cast<T*>(AddComponent(e, Type::GetTypeID<T>(), &temp));
    }

    void* GetComponent(Entity e, TypeID typeID) const;

    inline void* GetComponent(Entity e, const char* name) const
    {
        return GetComponent(e, Type::GetTypeIDByName(name));
    }

    template <typename T>
    T* GetComponent(Entity e)
    {
        return static_cast<T*>(GetComponent(e, Type::GetTypeID<T>()));
    }

    void* GetOrAddComponent(Entity e, TypeID typeID);

    inline void* GetOrAddComponent(Entity e, const char* name)
    {
        return GetOrAddComponent(e, Type::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetOrAddComponent(Entity e)
    {
        return static_cast<T*>(GetOrAddComponent(e, Type::GetTypeID<T>()));
    }

    void DelComponent(Entity e, Hash64 hash);

    void DelComponent(Entity e, const char* name);

    template <typename T>
    void DelComponent(Entity e)
    {
        DelComponent(e, GetHash64ByType<T>());
    }

    ArchetypeStorage* GetArchetype(Signature mask) const;

    template<typename... Components>
    View<Components...> Query()
    {
        return View<Components...>(this);
    }

    size_t GetCount() const;

private:
    std::shared_ptr<EntityLocationMap> m_entityLocationMap;

    std::vector<ArchetypeStorage*> m_entitesInfo;

    ArchetypeStorage* m_emptyArchetypeStorage;
};

