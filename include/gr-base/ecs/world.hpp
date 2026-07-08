#pragma once

#include "gr-base/ecs/archetype.hpp"

#include "gr-base/ecs/detail.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/externals/handle.h"

#include <unordered_map>
#include <memory>
#include <vector>

namespace engine::ecs
{
struct world
{
    std::unordered_map<Signature, ArchetypeID> signatureToArchetype;

    std::vector<engine::ecs::detail::records> entityRecords;

    engine::ecs::manager::entity entityManager;

    ArchetypeID m_emptyArchetypeStorage = INVALID_ARCHETYPE;

    archetype* m_archetypePool = nullptr;
    
    size_t m_capacity = 0;

    size_t m_count = 0;

    world();
    ~world();

    Entity Create();

    void Destroy(Entity entity);

    bool IsAlive(Entity entity) const;

    ArchetypeID GetOrCreateArcheType(Signature mask, TypeID typeID);

    Signature GetMask(Entity e) const;

    engine::ecs::detail::records Allocate(Entity entity, TypeID type);

    template <typename T, typename... Args>
    T* AddComponent(Entity entity, Args&&... args)
    {
        assert(entityManager.isAlive(entity));

        TypeID type = ComponentRegistry::GetTypeID<T>();
        assert(type != INVALID_TYPE);

        auto record = Allocate(entity, type);

        auto arche = GetArchetypeByID(record.archetype);

        T* component = arche->get<T>(record.location);
        new (component) T { std::forward<Args>(args)... };

        return component;
    }

    template <typename T>
    T* GetComponent(Entity entity) const
    {
        if (!entityManager.isAlive(entity))
            return nullptr;

        TypeID type = ComponentRegistry::GetTypeID<T>();
        if (type == INVALID_TYPE)
            return nullptr;

        uint32_t entityID = handle_get_first(entity);
        if (entityID > entityRecords.size())
            return nullptr;

        auto& record = entityRecords[entityID];
        assert(record.archetype != INVALID_ARCHETYPE);
        assert(record.mask != INVALID_SIGNATURE);

        Signature requiredMask =
            ComponentRegistry::GetMask<T>();
        if (
                record.archetype == INVALID_ARCHETYPE ||
                record.mask == INVALID_SIGNATURE ||
                ((record.mask & requiredMask) != requiredMask)
            )
            return nullptr;

        auto arche = GetArchetypeByID(record.archetype);

        return arche->get<T>(record.location);
    }

    template <typename... Components>
    bool HasComponent(Entity entity) const
    {
        if (!entityManager.isAlive(entity))
            return false;

        Signature requiredMask =
            ComponentRegistry::GetMask<Components...>();

        uint32_t entityID = handle_get_first(entity);
        assert(entityID < entityRecords.size());

        auto& record = entityRecords[entityID];
        assert(record.archetype != INVALID_ARCHETYPE);

        return record.mask != INVALID_SIGNATURE && (record.mask & requiredMask) == requiredMask;
    }

    void* AddComponentByID(Entity entity, TypeID type);

    void* GetComponentByID(Entity entity, TypeID type) const;

    /*
    template <typename T>
    void DelComponent(Entity e)
    {
        DelComponent(e, engine::type_traits::hash<T>);
    }
    */

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

            for (size_t j=0;j<arche.chunkCount;j++)
            {
                Chunk* chunk = arche.chunks[j];

                func(chunk->entities, arche.getRaw<Components>(static_cast<ChunkID>(j))..., chunk->count);
            }
        }
    }

    size_t GetEntityCount() const;

    archetype* GetArchetypeByID(ArchetypeID id) const;

    inline archetype* GetRawArchetype() const
    {
        return m_archetypePool;
    }

    inline ArchetypeID GetArchetypeCount() const
    {
        return m_count;
    }

private:

    engine::ecs::detail::records MoveEntity(engine::ecs::detail::records src_rec, ArchetypeID dstID, Entity e);

    void LinkAdd(TypeID typeID, ArchetypeID srcID, ArchetypeID dstID);

    void resize();
};

};
