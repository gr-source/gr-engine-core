#pragma once

#include "gr-base/ecs/archetype.hpp"

#include "gr-base/ecs/entityManager.hpp"
#include "gr-base/ecs/signature.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/detail.hpp"

#include <functional>

namespace engine::ecs
{
struct world
{
    std::unordered_map<Signature, ArchetypeID> signatureToArchetype;

    std::vector<std::vector<std::function<void(engine::ecs::world*, Entity)>>> events;

    std::vector<detail::records> entityRecords;

    manager::entity entityManager;

    ArchetypeID m_emptyArchetypeStorage = INVALID_ARCHETYPE;

    archetype* m_archetypePool = nullptr;
    
    size_t m_capacity = 0;

    size_t m_count = 0;

    world();
    ~world();

    template <typename Component, typename Callback, typename... Args>
    void AddOnCreate(Callback&& callback, Args&&... args);

    Entity Create();

    void Destroy(Entity entity);

    bool IsAlive(Entity entity) const;

    ArchetypeID GetOrCreateArcheType(Signature mask);

    Signature GetMask(Entity e) const;

    detail::records Allocate(Entity entity, Signature mask);

    detail::records Allocate(Entity entity, TypeID type);

    template <typename T, typename... Args>
    T* AddComponent(Entity entity, Args&&... args)
    {
        if (!entityManager.isAlive(entity))
            return nullptr;

        TypeID typeID =
            GetComponentID<T>::id;
        if (typeID == INVALID_TYPE)
            return nullptr;

        uint32_t entityID = handle_get_first(entity);
        if (entityID >= entityRecords.size())
            return nullptr;

        Signature requiredMask =
            GetComponentMask<T>::Get();

        auto& current = entityRecords[entityID];
        if (
                current.mask != INVALID_SIGNATURE &&
                ((current.mask & requiredMask) == requiredMask)
           )
        {
            auto arche = GetArchetypeByID(current.archetype);

            return arche->get<T>(current.location);
        }

        auto record = Allocate(entity, typeID);

        auto arche = GetArchetypeByID(record.archetype);

        T* component = arche->get<T>(record.location);
        new (component) T { std::forward<Args>(args)... };

        if (typeID < events.size())
        {
            for (auto& func : events[typeID])
                func(this, entity);
        }

        return component;
    }

    template <typename T>
    T* GetComponent(Entity entity) const
    {
        if (!entityManager.isAlive(entity))
            return nullptr;

        TypeID type =
            GetComponentID<T>::id;
        if (type == INVALID_TYPE)
            return nullptr;

        uint32_t entityID = handle_get_first(entity);
        if (entityID >= entityRecords.size())
            return nullptr;

        auto& record = entityRecords[entityID];
        assert(record.archetype != INVALID_ARCHETYPE);
        assert(record.mask != INVALID_SIGNATURE);

        Signature requiredMask =
            GetComponentMask<T>::Get();
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
    std::tuple<Components*...> GetComponents(Entity entity) const
    {
        Signature requiredMask =
            GetComponentMask<Components...>::Get();

        uint32_t entityID = handle_get_first(entity);

        auto& record = entityRecords[entityID];

        auto arche = GetArchetypeByID(record.archetype);

        return std::make_tuple(arche->get<Components>(record.location)...);
    }

    template <typename... Components>
    bool HasComponent(Entity entity) const
    {
        if (!entityManager.isAlive(entity))
            return false;

        Signature requiredMask =
            GetComponentMask<Components...>::Get();

        uint32_t entityID = handle_get_first(entity);
        assert(entityID < entityRecords.size());

        auto& record = entityRecords[entityID];
        assert(record.archetype != INVALID_ARCHETYPE);

        return record.mask != INVALID_SIGNATURE && (record.mask & requiredMask) == requiredMask;
    }

    void* AddComponentByID(Entity entity, TypeID type);

    void AddComponentByMask(Entity entity, Signature requiredMask);

    void* GetComponentByID(Entity entity, TypeID type) const;

    void DelComponentByID(Entity entity, TypeID type);

    void DelComponentByMask(Entity entity, Signature requiredMask);

    std::vector<TypeID> GetAllComponentType(Entity e) const;

    template<typename... Components, typename Func>
    void Query(Func&& func)
    {
        Signature requiredMask =
            GetComponentMask<Components...>::Get();

        for (size_t i=0;i<m_count;i++)
        {
            const auto& arche = m_archetypePool[i];

            if ((arche.mask & requiredMask) != requiredMask)
                continue;

            for (size_t j=0;j<arche.chunkCount;j++)
            {
                Chunk* chunk = arche.chunks[j];

                func(chunk->count, chunk->entities, arche.getRaw<Components>(static_cast<ChunkID>(j))...);
            }
        }
    }

    size_t GetEntityCount() const;

    archetype* GetArchetypeByID(ArchetypeID id) const;

    inline archetype* GetRawArchetype() const
    {
        return m_archetypePool;
    }

    inline size_t GetArchetypeCount() const
    {
        return m_count;
    }

    void RemoveArchetype(ArchetypeID id);

private:

    engine::ecs::detail::records MoveEntity(engine::ecs::detail::records src_rec, ArchetypeID dstID, Entity e);

    void LinkAdd(TypeID typeID, ArchetypeID srcID, ArchetypeID dstID);

    void resize();
};


template <typename Component, typename Callback, typename... Args>
inline void world::AddOnCreate(Callback&& callback, Args&&... args)
{
    auto arguments = std::make_tuple(std::forward<Args>(args)...);

    std::function<void(engine::ecs::world*, Entity)> function =
        [callback = std::forward<Callback>(callback),
         arguments = std::move(arguments)](engine::ecs::world* world, Entity entity) mutable
        {
            std::apply(
                [&](auto&&... unpacked)
                {
                    std::invoke(callback, unpacked..., world, entity);
                },
                arguments
            );
        };

    auto typeID =
        GetComponentID<Component>::id;
    if (typeID != INVALID_TYPE)
    {
        if (typeID >= events.size())
            events.resize(typeID + 1);

        events[typeID].push_back(function);
    }
}

};
