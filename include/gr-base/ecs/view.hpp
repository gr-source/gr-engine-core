#pragma once

#include "gr-base/ecs/archetypeStorage.hpp"
#include "gr-base/ecs/world.hpp"
#include "gr-base/ecs/type.hpp"

#include <tuple>

template<typename... Components>
class View
{
    class Iterator
    {
    public:

        View* m_view;

        size_t m_archetypeIndex;
        ComponentID m_localIndex;

        Iterator(
            View* view,
            size_t archetypeIndex,
            ComponentID localIndex
        )
            : m_view(view),
              m_archetypeIndex(archetypeIndex),
              m_localIndex(localIndex)
        {
            SkipEmpty();
        }

        void SkipEmpty()
        {
            while (m_archetypeIndex < m_view->m_matching.size())
            {
                if (m_localIndex < m_view->m_world->GetCount(m_view->m_matching[m_archetypeIndex]))
                    return;

                ++m_archetypeIndex;
                m_localIndex = 0;
            }
        }

        Iterator& operator++()
        {
            ++m_localIndex;
            SkipEmpty();
            return *this;
        }

        bool operator!=(const Iterator& other) const
        {
            return
                m_archetypeIndex != other.m_archetypeIndex ||
                m_localIndex != other.m_localIndex;
        }

        auto operator*()
        {
            auto* arch =
                m_view->m_world->GetArchetypeByID(m_view->m_matching[m_archetypeIndex]);

            Entity entity =
                arch->slotToEntity[m_localIndex];

            return std::tuple<
                Entity,
                Components*...
            >
            {
                entity,

                (
                    reinterpret_cast<Components*>(
                        arch->GetRaw(
                            Type::GetTypeID<Components>()
                        )
                    ) + m_localIndex
                )...
            };
        }
    };

public:
    std::vector<ArchetypeID> m_matching;

    World* m_world;

    ComponentID m_count;
public:

    View(World* world)
        : m_world(world), m_count(0)
    {
        Signature requiredMask = Type::GetMask<Components...>();

        ArchetypeStorage* array = world->GetRawArchetype();

        for (ArchetypeID i=0;i<world->GetArchetypeCount();i++)
        {
            if ((array[i].m_mask & requiredMask) == requiredMask)
            {
                m_matching.push_back(i);
                m_count += array[i].entityCount;
            }
        }
    }

    Iterator begin()
    {
        return Iterator(this, 0, 0);
    }

    Iterator end()
    {
        return Iterator(
            this,
            m_matching.size(),
            0
        );
    }
};

