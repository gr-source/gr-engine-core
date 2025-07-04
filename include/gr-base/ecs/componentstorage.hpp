#pragma once

#include "gr-base/ecs/icomponentstorage.hpp"
#include "gr-base/ecs/methodinvoke.hpp"

#include "gr-base/ecs/chunk.hpp"

#include "gr-base/ecs/component.hpp"

#include <stdexcept>
#include <cstring>
#include <string>
#include <vector>

#define MAX_COMPONENT_CHUNK 64

template <typename T>
using ComponentChunk = Chunk<T, MAX_COMPONENT_CHUNK>;

template <typename T>
class ComponentStorage : public IComponentStorage
{
    std::vector<std::unique_ptr<ComponentChunk<T>>> m_chunklist;

    std::vector<std::tuple<std::size_t, ComponentChunk<T> *>> m_freeslot;
    
    ComponentChunk<T> *m_lastchunk;

public:
    ComponentStorage() : m_lastchunk(nullptr) {}

    Component *allocate(Entity *entity, const Component *other) override
    {
        if (!m_freeslot.empty())
        {
            auto &&[id, chunk] = m_freeslot.back();
            m_freeslot.pop_back();

            chunk->usage[id] = true;
            chunk->activecount++;

            auto component = static_cast<Component *>(&chunk->data[id]);
            component = static_cast<Component *>(other == nullptr ? new (component) T() : new (component) T (*static_cast<const T *>(other)));

            component->entity = entity;

            return component;
        }

        if (m_lastchunk != nullptr)
        {
            for (std::size_t i={};i<MAX_COMPONENT_CHUNK;i++)
            {
                if (m_lastchunk->usage[i])
                    continue;

                m_lastchunk->usage[i] = true;
                m_lastchunk->activecount++;

                auto component = static_cast<Component *>(&m_lastchunk->data[i]);
                component = static_cast<Component *>(other == nullptr ? new (component) T() : new (component) T (*static_cast<const T *>(other)));
                
                component->entity = entity;

                return component;
            }
        }

        auto chunk = &(*m_chunklist.emplace_back(std::make_unique<ComponentChunk<T>>()));
        chunk->usage[0] = true;
        chunk->activecount++;

        m_lastchunk = chunk;

        auto component = static_cast<Component *>(&chunk->data[0]);
        component = static_cast<Component *>(other == nullptr ? new (component) T() : new (component) T (*static_cast<const T *>(other)));

        component->entity = entity;

        return component;
    }

    void deallocate(Component *component) override
    {
        for (auto &chunk : m_chunklist)
        {
            auto base = reinterpret_cast<std::uintptr_t>(chunk->data);
            auto target = reinterpret_cast<std::uintptr_t>(component);

            auto offset = (target - base) / sizeof(T);

            if (target >= base && offset <= MAX_COMPONENT_CHUNK)
            {
                chunk->usage[offset] = false;
                std::memset(component, 0, sizeof(T));

                m_freeslot.push_back(std::make_tuple(static_cast<std::size_t>(offset), chunk.get()));
                return;
            }
        }

        throw std::runtime_error("Fail do deallocate component");
    }

    template <typename R, typename ...Args>
    void register_method(std::string name, R (T::*method)(Args...))
    {
        constexpr auto argcount = sizeof...(Args);

        name += '_' + std::to_string(argcount);

        m_methodmap.emplace(name, std::make_unique<MethodInvoke<T, R, Args...>>(method));
    }
};


