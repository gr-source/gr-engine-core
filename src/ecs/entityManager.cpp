#include "gr-base/ecs/entityManager.hpp"

#include <cstdlib>

namespace engine::ecs
{

EntityManager::EntityManager() :
    freeID(INVALID_INDEX32),
    count(0),
    capacity(0),
    entities(nullptr)
{}

EntityManager::~EntityManager()
{
    if (entities != nullptr)
        free(entities);
}

Entity EntityManager::create()
{
    if (freeID == INVALID_INDEX32)
        grow();

    EntityID id = freeID;

    EntityInfo& info = entities[id];
    freeID = info.next;

    records[id] = {
        .mask = INVALID_SIGNATURE,
        .archetype = 0,

        .location = {
            .chunk = INVALID_CHUNK,
            .row = 0
        }
    };

    count++;

    return handle_create(id, info.generation);
}

void EntityManager::destroy(Entity entity)
{
    EntityID id = handle_get_first(entity);
    EntityID generation = handle_get_second(entity);

    EntityInfo& info = entities[id];
    if (info.generation != generation)
        return;

    records[id] = {
        .mask = INVALID_SIGNATURE,
        .archetype = 0,

        .location = {
            .chunk = INVALID_CHUNK,
            .row = 0
        }
    };

    info.next = freeID;
    freeID = id;

    info.generation++;

    count--;
}

bool EntityManager::isAlive(Entity entity) const
{
    EntityID id = handle_get_first(entity);
    EntityID generation = handle_get_second(entity);

    return entity != INVALID_ENTITY &&  entities[id].generation == generation;
}

// private
void EntityManager::grow()
{
    size_t oldCapacity = capacity;
    capacity = capacity > 0 ? capacity * 2 : 2;

    records =
        (detail::records*)realloc(records, capacity * sizeof(detail::records));
    entities =
        (EntityInfo*)realloc(entities, capacity * sizeof(EntityInfo));

    addFreeID(oldCapacity);
}

void EntityManager::addFreeID(size_t oldCapacity)
{
    for (size_t i=oldCapacity;i<capacity - 1;i++)
        entities[i] = { static_cast<EntityID>(i) + 1, 1 };

    entities[capacity - 1] = { freeID, 1 };
    freeID = static_cast<EntityID>(oldCapacity);
}

}
