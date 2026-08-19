#pragma once

#include "gr-base/ecs/default.hpp"
#include "gr-base/ecs/detail.hpp"

namespace engine::ecs
{

struct EntityManager
{
    typedef struct EntityInfo
    {
        uint32_t next;
        uint32_t generation;
    } EntityInfo;

    detail::records* records = nullptr;
    EntityInfo *entities = nullptr;

    size_t capacity;
    size_t count;

    EntityID freeID;

    EntityManager();
    ~EntityManager();

    Entity create();

    void destroy(Entity entity);

    bool isAlive(Entity entity) const;

private:
    void grow();

    void addFreeID(size_t oldCapacity);
};
}

