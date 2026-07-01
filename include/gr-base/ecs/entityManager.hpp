#pragma once

#include "gr-base/ecs/default.hpp"

namespace engine::ecs::manager
{

struct entity
{
    typedef struct EntityInfo
    {
        uint32_t next;
        uint32_t generation = 1;
    } EntityInfo;

    EntityInfo *entities = nullptr;

    size_t capacity = 0;
    size_t count = 0;

    EntityID freeID = 0;

    ~entity();

    Entity create();

    void destroy(Entity entity);

    bool isAlive(Entity entity) const;

    void grow();

    void addFreeID(size_t oldCapacity);
};
}

