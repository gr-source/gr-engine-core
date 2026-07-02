#include "gr-base/ecs/entityManager.hpp"

#include <iostream>
#include <cstdlib>

namespace engine::ecs::manager
{

entity::entity() : freeID(INVALID_INDEX32), count(0), capacity(0), entities(nullptr)
{
}

entity::~entity()
{
    if (entities != nullptr)
        free(entities);
}

Entity entity::create()
{
    if (freeID == INVALID_INDEX32)
        grow();

    EntityID id = freeID;

    EntityInfo& info = entities[id];
    freeID = info.next;

    count++;

    return handle_create(id, info.generation);
}

void entity::destroy(Entity entity)
{
    EntityID id = handle_get_first(entity);
    EntityID generation = handle_get_second(entity);

    EntityInfo& info = entities[id];
    if (info.generation != generation)
        return;

    info.next = freeID;
    freeID = id;

    info.generation++;

    count--;
}

bool entity::isAlive(Entity entity) const
{
    EntityID id = handle_get_first(entity);
    EntityID generation = handle_get_second(entity);

    return entity != INVALID_ENTITY &&  entities[id].generation == generation;
}

// private
void entity::grow()
{
    size_t oldCapacity = capacity;
    capacity = capacity > 0 ? capacity * 2 : 2;

    entities = (EntityInfo*)realloc(entities, capacity * sizeof(EntityInfo));

    addFreeID(oldCapacity);
}

void entity::addFreeID(size_t oldCapacity)
{
    for (size_t i=oldCapacity;i<capacity - 1;i++)
        entities[i] = { static_cast<EntityID>(i) + 1, 1 };

    entities[capacity - 1] = { freeID, 1 };
    freeID = static_cast<EntityID>(oldCapacity);
}

}
