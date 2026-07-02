#pragma once

#include "gr-base/ecs/dynamic/archetype.hpp"

#include "gr-base/ecs/detail.hpp"
#include "gr-base/ecs/default.hpp"

#include <unordered_map>
#include <memory>
#include <vector>

namespace engine::ecs::dynamic
{
struct world : engine::ecs::detail::world_base<engine::ecs::dynamic::archetype>
{
    world();
    ~world();

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
        return reinterpret_cast<T*>(GetComponent(e, ComponentRegistry::GetTypeID<T>()));
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
        return;
        Signature requiredMask = ComponentRegistry::GetMask<Components...>();

        for (size_t i=0;i<m_count;i++)
        {
            const auto& arche = m_archetypePool[i];

            if ((arche.mask & requiredMask) != requiredMask)
                continue;

            size_t chunk_count = arche.layout->capacity > 0 ? (arche.entityCount + arche.layout->capacity - 1) / arche.layout->capacity : 0;
            for (size_t j=0;j<chunk_count;j++)
            {
                Chunk* chunk = arche.chunks[j];

                Entity* entities = (Entity*)(chunk + 1);

                func(entities, arche.getRaw<Components>(static_cast<ChunkID>(j))..., chunk->count);
            }
        }
    }

    size_t GetEntityCount() const;

    engine::ecs::dynamic::archetype* GetArchetypeByID(ArchetypeID id) const;

    inline engine::ecs::dynamic::archetype* GetRawArchetype() const
    {
        return m_archetypePool;
    }

    inline ArchetypeID GetArchetypeCount() const
    {
        return m_count;
    }

private:

    void MoveEntity(ArchetypeID srcID, ArchetypeID dstID, Entity e);

    void LinkAdd(TypeID typeID, ArchetypeID srcID, ArchetypeID dstID);

    void resize();
};

};
