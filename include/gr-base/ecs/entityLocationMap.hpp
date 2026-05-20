#pragma once

#include "gr-base/ecs/default.hpp"

#include <vector>

class EntityLocationMap
{
    EntityID m_capacity;
    EntityID m_count;

    std::vector<uint32_t> m_generations;
    std::vector<uint32_t> m_freeIDs;

public:
    EntityLocationMap();

    Entity create();

    void destroy(Entity e);

    bool isAlive(Entity h) const;

    inline size_t GetCount() const
    {
        return m_count;
    }
};
