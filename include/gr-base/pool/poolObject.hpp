#pragma once

#include <cstdlib>
#include <utility>

#include <stdint.h>
#include <string.h>

#include "gr-base/externals/handle.h"

#define INVALID_ID (PoolID)-1

typedef uint32_t PoolID;

#include <iostream>
template <typename T>
class ObjectPool
{
public:
    ObjectPool() : m_slotCapacity(0), m_denseCount(0), m_denseCapacity(0), m_dense(NULL), m_generations(NULL), m_sparse(nullptr), m_freeList(nullptr), m_freeID(INVALID_ID), m_poolIDToDense(nullptr), m_denseIDToPoolID(nullptr)
    {}

    ~ObjectPool()
    {
        if (m_dense != NULL)
            free(m_dense);

        if (m_generations != NULL)
            free(m_generations);

        if (m_sparse != nullptr)
            free(m_sparse);

        if (m_freeList != nullptr)
            free(m_freeList);

        if (m_poolIDToDense != nullptr)
            free(m_poolIDToDense);

        if (m_denseIDToPoolID != nullptr)
            free(m_denseIDToPoolID);
    }

    template <typename... Args>
    Handle create(Args&&... args)
    {
        Handle h = allocate();

        T* ptr = get(h);
        new (ptr) T(std::forward<Args>(args)...);

        return h;
    }

    void destroy(Handle h)
    {
        // T* ptr = get(id);
        // ptr->~T();

        deallocate(h);
    }

    inline T* get(Handle h) const
    {
        uint32_t slotID = handle_get_first(h);
        uint32_t gen = handle_get_second(h);

        if (gen != m_generations[slotID])
            return nullptr;

        return &m_dense[m_poolIDToDense[slotID]];
    }

    inline T* getRaw() const
    {
        return m_dense;
    }

    inline size_t getCount() const
    {
        return m_denseCount;
    }

private:
    struct Slot
    {
        uint32_t index;
        uint32_t gen;
    };

    size_t m_slotCapacity;

    uint32_t* m_generations;
    PoolID* m_sparse;

    size_t m_denseCapacity;
    size_t m_denseCount;
    T* m_dense;

    PoolID* m_freeList;
    PoolID m_freeID;

    uint32_t* m_poolIDToDense;
    uint32_t* m_denseIDToPoolID;

    Handle allocate()
    {
        if (m_freeID == INVALID_ID)
            grow_slot();

        uint32_t slotID = m_freeID;
        m_freeID = m_freeList[slotID];

        uint32_t index = static_cast<uint32_t>(m_denseCount++);
        if (index >= m_denseCapacity)
            grow_dense();

        m_poolIDToDense[slotID] = index;
        m_denseIDToPoolID[index] = slotID;

        uint32_t gen = m_generations[slotID];

        return handle_create(slotID, gen);
    }

    void deallocate(Handle h)
    {
        uint32_t slotID = handle_get_first(h);
        uint32_t gen = handle_get_second(h);
        if (gen != m_generations[slotID])
            return;

        PoolID id = m_poolIDToDense[slotID];
        T& data = m_dense[id];
        data.~T();

        PoolID last = static_cast<PoolID>(--m_denseCount);
        if (id != last)
        {
            T* src = m_dense + last;
            T* dst = m_dense + id;

            new (dst) T(std::move(*src));

            m_denseIDToPoolID[id] = m_denseIDToPoolID[last];

            m_poolIDToDense[m_denseIDToPoolID[last]] = id;
        }

        m_generations[slotID]++;

        m_freeList[slotID] = m_freeID;
        m_freeID = slotID;
    }

    void grow_slot()
    {
        size_t oldCapacity = m_slotCapacity;
        m_slotCapacity = m_slotCapacity > 0 ? m_slotCapacity * 2 : 2;

        m_poolIDToDense = (PoolID*)realloc(m_poolIDToDense, m_slotCapacity * sizeof(PoolID));
        m_generations = (uint32_t*)realloc(m_generations, m_slotCapacity * sizeof(uint32_t));

        m_freeList = (PoolID*)realloc(m_freeList, m_slotCapacity * sizeof(PoolID));

        for (size_t i=oldCapacity;i<m_slotCapacity;i++)
            m_generations[i] = 1;

        addFreeList(oldCapacity);
    }

    void grow_dense()
    {
        size_t oldCapacity = m_denseCapacity;

        m_denseCapacity = m_denseCapacity > 0 ? m_denseCapacity * 2 : 2;

        if (m_dense != nullptr)
        {
            T* array = (T*)malloc(m_denseCapacity * sizeof(T));
            for (size_t i=0;i<m_denseCount;i++)
            {
                T* src = m_dense + i;
                T* dst = array + i;

                new (dst) T(std::move(*src));
                src->~T();
            }

            free(m_dense);
            m_dense = array;
        } else
        {
            m_dense = (T*)malloc(m_denseCapacity * sizeof(T));
        }

        m_denseIDToPoolID = (PoolID*)realloc(m_denseIDToPoolID, m_denseCapacity * sizeof(PoolID));
    }

    void addFreeList(size_t oldCapacity)
    {
        for (size_t i=oldCapacity;i<m_slotCapacity - 1;i++)
            m_freeList[i] = static_cast<PoolID>(i + 1);

        m_freeList[m_slotCapacity - 1] = m_freeID;
        m_freeID = static_cast<PoolID>(oldCapacity);
    }
};
