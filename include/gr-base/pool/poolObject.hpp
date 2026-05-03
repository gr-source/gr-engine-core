#pragma once

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

    PoolID allocate()
    {
        if (m_freeID == INVALID_ID)
            realloc();

        PoolID id = m_freeID;
        m_freeID = ((PoolID*)(m_slotID->vector))[id];

        return id;
    }

    void deallocate(PoolID id)
    {
        ((PoolID*)(m_slotID->vector))[id] = m_freeID;
        m_freeID = id;
    }

    inline T* get(PoolID id) const
    {
        return &reinterpret_cast<T*>(m_dense->vector)[id];
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

    void realloc()
    {
        PoolID oldCapacity = m_capacity;

        if (m_capacity)
        {
            m_capacity *= 2;

            m_slotID = array_resize(m_slotID, m_capacity, sizeof(PoolID));

            PoolID* slotIDs = (PoolID*)(m_slotID->vector);
            memset(&slotIDs[oldCapacity], INVALID_ID, oldCapacity * sizeof(PoolID));

            m_dense = array_resize(m_dense, m_capacity, sizeof(T));

            T* dense = (T*)(m_dense->vector);
            memset(&dense[oldCapacity], 0, oldCapacity * sizeof(T));
        } else
        {
            m_capacity = 1;

            m_slotID = array_create(m_capacity, sizeof(PoolID));
            memset(m_slotID->vector, INVALID_ID, m_capacity * sizeof(PoolID));

            m_dense = array_create(m_capacity, sizeof(T));
            memset(m_dense->vector, 0, m_capacity * sizeof(T));
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

