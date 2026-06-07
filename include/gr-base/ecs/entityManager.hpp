#pragma once

#include "gr-base/ecs/default.hpp"

class EntityManager
{
public:
    EntityManager();
    ~EntityManager();

    Entity create();

    void destroy(Entity entity);

    bool isAlive(Entity entity) const;

    inline EntityID GetCount() const
    {
        return m_count;
    }

private:
    struct EntityInfo
    {
        uint32_t next;
        uint32_t generation;
    };

    EntityInfo *m_entities;

    size_t m_capacity;
    size_t m_count;

    EntityID m_freeID;

    void resize();

    void addFreeID(size_t oldCapacity);
};
