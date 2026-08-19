#pragma once

#include "gr-base/ecs/archetype.hpp"

#include "gr-base/ecs/entityManager.hpp"
#include "gr-base/ecs/signature.hpp"
#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/detail.hpp"

#include <unordered_map>
#include <vector>

namespace engine::ecs
{
template<typename... Components>
class View;

struct ArchetypeStorage
{
    archetype* archetypes = nullptr;
    graph* graphs = nullptr;

    size_t capacity = 0;
    size_t count = 0;
};

class World
{
    std::unordered_map<Signature, ArchetypeID> signatureToArchetype;

    // std::vector<detail::records> entityRecords;

    EntityManager entityManager;

    ArchetypeID m_emptyArchetypeStorage = INVALID_ARCHETYPE;

    ArchetypeStorage storage;

public:
    World();
    ~World();

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    World(World&&) = default;
    World& operator=(World&&) = default;

    Entity Create();

    void Destroy(Entity entity);

    bool IsAlive(Entity entity) const;

    ArchetypeID GetOrCreateArcheType(Signature mask);

    Signature GetMask(Entity e) const;

    detail::records Allocate(Entity entity, Signature mask);

    detail::records Allocate(Entity entity, TypeID type);

    template <typename T, typename... Args>
    inline T* AddComponent(Entity entity, Args&&... args)
    {
        if (!entityManager.isAlive(entity))
            return nullptr;

        TypeID typeID =
            GetComponentID<T>::id;
        if (typeID == INVALID_TYPE)
            return nullptr;

        uint32_t entityID = handle_get_first(entity);

        Signature requiredMask =
            GetComponentMask<T>::Get();

        auto& current = entityManager.records[entityID];
        if (
                current.mask != INVALID_SIGNATURE &&
                ((current.mask & requiredMask) == requiredMask)
           )
        {
            auto arche = GetArchetypeByID(current.archetype);

            return arche->get<T>(current.location);
        }

        auto record = Allocate(entity, typeID);
        if (engine::type_traits::tag<T>)
            return nullptr;

        auto arche = GetArchetypeByID(record.archetype);

        T* component = arche->get<T>(record.location);
        new (component) T { std::forward<Args>(args)... };

        return component;
    }

    template <typename T>
    T& GetComponent(Entity entity) const
    {
        static_assert(!engine::type_traits::tag<T>,
              "All template arguments must be Tags.");

        uint32_t entityID = handle_get_first(entity);

        auto& record = entityManager.records[entityID];
        assert(record.archetype != INVALID_ARCHETYPE);
        assert(record.mask != INVALID_SIGNATURE);

        Signature requiredMask =
            GetComponentMask<T>::Get();

        assert((record.mask & requiredMask) == requiredMask);

        auto arche = GetArchetypeByID(record.archetype);

        return *arche->get<T>(record.location);
    }

    template <typename T>
    T* TryGetComponent(Entity entity) const
    {
        static_assert(!engine::type_traits::tag<T>,
              "All template arguments must be Tags.");

        if (!entityManager.isAlive(entity))
            return nullptr;

        TypeID type =
            GetComponentID<T>::id;
        if (type == INVALID_TYPE)
            return nullptr;

        uint32_t entityID = handle_get_first(entity);

        auto& record = entityManager.records[entityID];
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
        static_assert((!engine::type_traits::tag<Components> && ...), 
              "All template arguments must be Tags.");

        uint32_t entityID = handle_get_first(entity);

        auto& record = entityManager.records[entityID];

        auto arche = GetArchetypeByID(record.archetype);

        return std::make_tuple(arche->get<Components>(record.location)...);
    }

    template <typename... Components>
    bool HasComponent(Entity entity) const
    {
        Signature requiredMask =
            GetComponentMask<Components...>::Get();

        uint32_t entityID = handle_get_first(entity);

        auto& record = entityManager.records[entityID];
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

        for (size_t i=0;i<storage.count;i++)
        {
            const auto& arche = storage.archetypes[i];

            if ((arche.mask & requiredMask) != requiredMask)
                continue;

            size_t* offsets =
                arche.layout->offsets;

            for (size_t j=0;j<arche.chunkCount;j++)
            {
                Chunk* chunk = arche.chunks[j];

                func(
                        chunk->count,
                        chunk->entities,
                        reinterpret_cast<Components*>(chunk->base + offsets[GetComponentID<Components>::id])...
                    );
            }
        }
    }

    template <typename... Components>
    inline View<Components...> view()
    {
        return View<Components...>(this);
    }

    size_t GetEntityCount() const;

    archetype* GetArchetypeByID(ArchetypeID id) const;

    ArchetypeStorage& GetStorage();

private:

    engine::ecs::detail::records MoveEntity(engine::ecs::detail::records src_rec, ArchetypeID dstID, Entity e);

    void LinkAdd(ArchetypeID src, TypeID typeID, ArchetypeID dst);

    ArchetypeID AddEdges(ArchetypeID src, TypeID typeID);

    void resize();
};
};
