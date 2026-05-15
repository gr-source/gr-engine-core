#pragma once

#include <cstdlib>
#include <utility>
extern "C"
{
#include "gr-base/externals/array.h"
}

#include <stdint.h>
#include <string.h>

#define INVALID_ID (PoolID)-1

typedef uint32_t PoolID;

template <typename T>
class ObjectPool
{
public:
    ObjectPool() : m_freeID(INVALID_ID), m_capacity(0), m_dense(NULL), m_slotID(NULL)
    {}

    ~ObjectPool()
    {
        if (m_dense != NULL)
            array_free(m_dense);

        if (m_slotID != NULL)
            array_free(m_slotID);
    }

    template <typename... Args>
    PoolID create(Args&&... args)
    {
        PoolID id = allocate();

        T* ptr = get(id);
        new (ptr) T(std::forward<Args>(args)...);

        return id;
    }

    void destroy(PoolID id)
    {
        T* ptr = get(id);
        ptr->~T();

        deallocate(id);
    }

    PoolID allocate()
    {
        if (m_freeID == INVALID_ID)
            resize();

        PoolID id = m_freeID;
        m_freeID = ((PoolID*)(m_slotID->vector))[id];

        m_alive[id] = true;

        return id;
    }

    void deallocate(PoolID id)
    {
        ((PoolID*)(m_slotID->vector))[id] = m_freeID;
        m_freeID = id;

        m_alive[id] = false;
    }

    inline T* get(PoolID id) const
    {
        return &reinterpret_cast<T*>(m_dense->vector)[id];
    }

    inline T* getRaw() const
    {
        return reinterpret_cast<T*>(m_dense->vector);
    }

    inline PoolID getCapacity() const
    {
        return m_capacity;
    }

private:
    PoolID m_capacity;

    PoolID m_freeID;

    array* m_slotID;

    array* m_dense;

    bool* m_alive;

    void resize()
    {
        PoolID oldCapacity = m_capacity;

        if (m_capacity)
        {
            m_capacity *= 2;

            m_alive = (bool*)realloc(m_alive, m_capacity * sizeof(bool));
            memset(&m_alive[oldCapacity], 0, oldCapacity * sizeof(bool));

            m_slotID = array_resize(m_slotID, m_capacity, sizeof(PoolID));

            PoolID* slotIDs = (PoolID*)(m_slotID->vector);
            memset(&slotIDs[oldCapacity], INVALID_ID, oldCapacity * sizeof(PoolID));

            array* pool = array_create(m_capacity, sizeof(T));

            T* newDense = (T*)(pool->vector);
            T* oldDense = (T*)(m_dense->vector);

            for (PoolID id=0;id<oldCapacity;id++)
            {
                if (!m_alive[id])
                    continue;

                new (&newDense[id]) T(std::move(oldDense[id]));

                oldDense[id].~T();
            }

            memset(&newDense[oldCapacity], 0, oldCapacity * sizeof(T));

            array_free(m_dense);
            m_dense = pool;
        } else
        {
            m_capacity = 1;

            m_slotID = array_create(m_capacity, sizeof(PoolID));
            memset(m_slotID->vector, INVALID_ID, m_capacity * sizeof(PoolID));

            m_dense = array_create(m_capacity, sizeof(T));
            memset(m_dense->vector, 0, m_capacity * sizeof(T));

            m_alive = (bool*)calloc(m_capacity, sizeof(bool));
        }

        addFreeID(oldCapacity);
    }

    void addFreeID(PoolID first)
    {
        PoolID* slotIDs = (PoolID*)(m_slotID->vector);

        PoolID capacity = m_capacity - 1;
        for (PoolID id=first;id<capacity;id++)
            slotIDs[id] = id + 1;

        slotIDs[capacity] = m_freeID;
        m_freeID = first;
    }
};

