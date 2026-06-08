#pragma once

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/type.hpp"

#include <unordered_map>
#include <memory>
#include <vector>

class World
{
public:
    World();
    ~World();

    Entity Create();

    void Destroy(Entity entity);

    bool IsAlive(Entity entity) const;

    ArchetypeID GetOrCreateArcheType(Signature mask, TypeID typeID);

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

    bool HasComponent(Entity entity, Signature mask) const;

    template <typename... Components>
    bool HasComponent(Entity entity) const
    {
        return HasComponent(entity, Type::GetMask<Components...>());
    }

    void DelComponent(Entity e, Hash64 hash);

    void DelComponent(Entity e, const char* name)
    {
        DelComponent(e, hash::fnv1_str(name));
    }

    template <typename T>
    void DelComponent(Entity e)
    {
        DelComponent(e, GetHash64ByType<T>());
    }

    std::vector<TypeID> GetAllComponentType(Entity e) const;

    template<typename... Components, typename Func>
    void Query(Func&& func)
    {
        Signature requiredMask = Type::GetMask<Components...>();

        for (size_t i=0;i<m_capacity;i++)
        {
            const auto& arche = m_archetypePool[i];

            if ((arche.mask & requiredMask) != requiredMask)
                continue;

            for (size_t j=0;j<arche.chunk_capacity;j++)
            {
                Chunk& chunk = arche.chunks[j];

                func(chunk.entities, arche.getRaw<Components>(static_cast<ChunkID>(j))..., chunk.count);
            }
        }
    }

    size_t GetEntityCount() const;

    ArchetypeStorage* GetArchetypeByID(ArchetypeID id) const;

    inline ArchetypeStorage* GetRawArchetype() const
    {
        return m_archetypePool;
    }

    inline ArchetypeID GetArchetypeCount() const
    {
        return m_count;
    }

private:
    struct EntityRecord
    {
        uint32_t archetype;

        EntityLocation location;
    };

    std::unordered_map<Signature, ArchetypeID> m_signArcheTypeMap;

    std::shared_ptr<EntityManager> m_entityLocationMap;

    std::vector<EntityRecord> entityRecords;

    ArchetypeID m_emptyArchetypeStorage;

    ArchetypeStorage* m_archetypePool;

    ArchetypeID m_capacity;

    ArchetypeID m_count;

    ArchetypeStorage* GetArchetypeByMask(Signature mask) const;

    void MoveEntity(ArchetypeID srcID, ArchetypeID dstID, Entity e);

    void LinkAdd(TypeID typeID, ArchetypeID srcID, ArchetypeID dstID);

    void resize();
};

