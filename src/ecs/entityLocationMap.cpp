#include "gr-base/ecs/entityLocationMap.hpp"

EntityLocationMap::EntityLocationMap() : m_count(0), m_capacity(0) {}

Entity EntityLocationMap::create()
{
    uint32_t id;
    if (!m_freeIDs.empty())
    {
        id = m_freeIDs.back();
        m_freeIDs.pop_back();
    } else
    {
        id = m_count++;
    }

    if (id >= m_capacity)
    {
        m_capacity = m_capacity > 0 ? m_capacity * 2 : 2;

        m_generations.resize(m_capacity, 1);
    }

    return handle_create(id, m_generations[id]);
}

void EntityLocationMap::destroy(Entity e)
{
    uint32_t id = handle_get_first(e);
    uint32_t version = handle_get_second(e);

    if (m_generations[id] != version)
    {
        return;
    }

    m_generations[id]++;

    m_freeIDs.push_back(id);
}

bool EntityLocationMap::isAlive(Entity h) const
{
    uint32_t entityID = handle_get_first(h);
    return handle_get_second(h) == m_generations[entityID];
}

