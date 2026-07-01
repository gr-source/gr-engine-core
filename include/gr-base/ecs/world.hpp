#pragma once

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/default.hpp"

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
        return AddComponent(e, ComponentRegistry::GetTypeIDByName(name), src);
    }

    template <typename T>
    T* AddComponent(Entity e, T temp ={})
    {
        return static_cast<T*>(AddComponent(e, ComponentRegistry::GetTypeID<T>(), &temp));
    }

    void* GetComponent(Entity e, TypeID typeID) const;

    inline void* GetComponent(Entity e, const char* name) const
    {
        return GetComponent(e, ComponentRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    T* GetComponent(Entity e)
    {
        return static_cast<T*>(GetComponent(e, ComponentRegistry::GetTypeID<T>()));
    }

    void* GetOrAddComponent(Entity e, TypeID typeID);

    inline void* GetOrAddComponent(Entity e, const char* name)
    {
        return GetOrAddComponent(e, ComponentRegistry::GetTypeIDByName(name));
    }

    template <typename T>
    inline T* GetOrAddComponent(Entity e)
    {
        return static_cast<T*>(GetOrAddComponent(e, ComponentRegistry::GetTypeID<T>()));
    }

    bool HasComponent(Entity entity, Signature mask) const;

    template <typename... Components>
    bool HasComponent(Entity entity) const
    {
        return HasComponent(entity, ComponentRegistry::GetMask<Components...>());
    }

    void DelComponent(Entity e, Hash64 hash);

    void DelComponent(Entity e, const char* name)
    {
        DelComponent(e, engine::hash::fnv1_str(name));
    }

    template <typename T>
    void DelComponent(Entity e)
    {
        DelComponent(e, engine::type_traits::hash<T>);
    }

    std::vector<TypeID> GetAllComponentType(Entity e) const;

    template<typename... Components, typename Func>
    void Query(Func&& func)
    {
        Signature requiredMask = ComponentRegistry::GetMask<Components...>();

        for (size_t i=0;i<m_count;i++)
        {
            const auto& arche = m_archetypePool[i];

            if ((arche.mask & requiredMask) != requiredMask)
                continue;

            size_t chunk_count = arche.layout->capacity > 0 ? (arche.entityCount + arche.layout->capacity - 1) / arche.layout->capacity : 0;
            for (size_t j=0;j<chunk_count;j++)
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

        engine::ecs::detail::location location;
    };

    std::unordered_map<Signature, ArchetypeID> m_signArcheTypeMap;

    std::shared_ptr<EntityManager> m_entityLocationMap;

    std::vector<EntityRecord> entityRecords;

    ArchetypeID m_emptyArchetypeStorage;

    ArchetypeStorage* m_archetypePool;

    size_t m_capacity;

    size_t m_count;

    ArchetypeStorage* GetArchetypeByMask(Signature mask) const;

    void MoveEntity(ArchetypeID srcID, ArchetypeID dstID, Entity e);

    void LinkAdd(TypeID typeID, ArchetypeID srcID, ArchetypeID dstID);

    void resize();
};



