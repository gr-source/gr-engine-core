#pragma once

#include "gr-base/ecs/default.hpp"

class EntityManager
{
    struct EntityInfo
    {
        uint32_t next;
        uint32_t generation;
    };

    EntityInfo *m_entities;

    EntityID m_capacity;
    EntityID m_count;

    EntityID m_freeID;

public:
    EntityManager();
    ~EntityManager();

    Entity create();

    void destroy(Entity e);

    bool isAlive(Entity h) const;

    void resize();

    void addFreeID(EntityID oldCapacity);

    inline EntityID GetCount() const
    {
        return m_count;
    }
};
