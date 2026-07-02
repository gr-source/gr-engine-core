#pragma once

#include "gr-base/ecs/default.hpp"

namespace engine::ecs::manager
{

struct entity
{
    typedef struct EntityInfo
    {
        uint32_t next;
        uint32_t generation;
    } EntityInfo;

    EntityInfo *entities;

    size_t capacity;
    size_t count;

    EntityID freeID;

    entity();
    ~entity();

    Entity create();

    void destroy(Entity entity);

    bool isAlive(Entity entity) const;

    void grow();

    void addFreeID(size_t oldCapacity);
};
}

