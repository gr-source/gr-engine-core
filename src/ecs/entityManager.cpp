#include "gr-base/ecs/entityManager.hpp"

#include <cstdlib>

EntityManager::EntityManager() : m_freeID(0), m_count(0), m_capacity(0), m_entities(nullptr)
{}

EntityManager::~EntityManager()
{
    if (m_entities != nullptr)
        free(m_entities);
}

Entity EntityManager::create()
{
    if (m_count >= m_capacity)
        resize();

    EntityID id = m_freeID;

    EntityInfo& info = m_entities[id];
    m_freeID = info.next;

    m_count++;

    return handle_create(id, info.generation);
}

void EntityManager::destroy(Entity entity)
{
    EntityID id = handle_get_first(entity);
    EntityID generation = handle_get_second(entity);

    EntityInfo& info = m_entities[id];
    if (info.generation != generation)
        return;

    info.next = m_freeID;
    m_freeID = id;

    info.generation++;

    m_count--;
}

bool EntityManager::isAlive(Entity entity) const
{
    EntityID id = handle_get_first(entity);
    EntityID generation = handle_get_second(entity);

    return entity != INVALID_ENTITY && m_entities[id].generation == generation;
}

void EntityManager::resize()
{
    EntityID oldCapacity = m_capacity;
    m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

    m_entities = (EntityInfo*)realloc(m_entities, m_capacity * sizeof(EntityInfo));

    addFreeID(oldCapacity);
}

void EntityManager::addFreeID(EntityID start)
{
    EntityID lastEntity = m_capacity - 1;
    for (EntityID i=start;i<lastEntity;i++)
        m_entities[i] = { i + 1, 1 };

    m_entities[lastEntity] = { m_freeID, 1 };
    m_freeID = start;
}


