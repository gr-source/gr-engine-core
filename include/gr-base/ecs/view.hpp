#pragma once

namespace engine::ecs
{
template<typename... Components>
class View
{
    using World = engine::ecs::world;

    World* world;
public:

    View(World* world) :
        world(world),
        m_requiredMask(GetComponentMask<Components...>::Get()),
        m_excludedMask(0)
    {}

    template<typename... Excluded>
    View& Without()
    {
        m_excludedMask =
            GetComponentMask<Excluded...>::Get();

        return *this;
    }

    template<typename Func>
    void Each(Func&& func)
    {
        for (size_t i=0;i<world->GetArchetypeCount();i++)
        {
            const auto& arche = world->GetRawArchetype()[i];

            if ((arche.mask & m_requiredMask) != m_requiredMask)
                continue;

            if ((arche.mask & m_excludedMask) != 0)
                continue;

            size_t* offsets =
                arche.layout->offsets;

            for (size_t j=0;j<arche.chunkCount;j++)
            {
                Chunk* chunk = arche.chunks[j];

                func(
                        chunk->count,
                        chunk->entities,
                        reinterpret_cast<Components*>(chunk->base + offsets[GetComponentID<Components>::id])...
                    );
            }
        }
    }

private:
    Signature m_requiredMask;
    Signature m_excludedMask;
};
};

