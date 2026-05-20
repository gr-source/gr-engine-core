#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/type.hpp"

#include <unordered_map>
#include <memory>
#include <vector>

class ECS
{
public:
    ECS();
    ~ECS();

    Entity Create();

    void Destroy(Entity e);

    bool IsAlive(Entity e) const;

    std::vector<TypeID> intersection(ArchetypeStorage* src, ArchetypeStorage* dst);

    void MoveEntity(ArcheTypeID srcID, ArcheTypeID dstID, Entity e);

    ArcheTypeID GetOrCreateArcheType(Signature mask, TypeID typeID);

    Signature GetMask(Entity e) const;

    void* AddComponent(Entity e, TypeID typeID, const void* component);

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

    std::vector<TypeID> GetAllComponentType(Entity e) const;

    template<typename... Components>
    View<Components...> Query()
    {
        return View<Components...>(this);
    }

    size_t GetCount(ArcheTypeID id) const;

    size_t GetCount() const;

    ArchetypeStorage* GetArchetypeByID(ArcheTypeID id) const;

    inline ArchetypeStorage* GetRawArcheType() const
    {
        return m_archetypePool;
    }

    inline ArcheTypeID GetArcheTypeCount() const
    {
        return m_count;
    }

private:
    std::unordered_map<Signature, ArcheTypeID> m_signArcheTypeMap;

    std::shared_ptr<EntityLocationMap> m_entityLocationMap;

    std::vector<ArcheTypeID> m_entitesInfo;

    ArcheTypeID m_emptyArchetypeStorage;

    ArchetypeStorage* m_archetypePool;

    ArcheTypeID m_capacity;

    ArcheTypeID m_count;

    ArchetypeStorage* GetArchetypeByMask(Signature mask) const;

    void LinkAdd(TypeID typeID, ArcheTypeID srcID, ArcheTypeID dstID);

    void resize();
};

