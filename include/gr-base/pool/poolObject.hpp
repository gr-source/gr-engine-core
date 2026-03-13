#pragma once

#include <cstring>
#include <stdint.h>

extern "C"
{
#include "gr-base/externals/array.h"
}

#define INVALID_POOL (PoolID)-1

typedef uint32_t PoolID;

template <typename T>
struct PoolSlot
{
    PoolID next;
    T element;
};

template <typename T>
class ObjectPool
{
public:
    ObjectPool();
    ~ObjectPool();

    PoolID allocate();

    void deallocate(PoolID id);

    T* get(PoolID id);

private:
    PoolID m_capacity;

    PoolID m_freeID;

    array* m_pool;

    void realloc();

    void addFreeID(PoolID first);
};

template <typename T>
inline ObjectPool<T>::ObjectPool() : m_freeID(INVALID_POOL), m_capacity(0), m_pool(nullptr)
{}

template <typename T>
inline ObjectPool<T>::~ObjectPool()
{
    if (m_pool != nullptr)
        array_free(m_pool);
}

template <typename T>
inline PoolID ObjectPool<T>::allocate()
{
    if (m_freeID == INVALID_POOL)
        realloc();

    PoolSlot<T>* wrapper = (PoolSlot<T>*)m_pool->vector;

    PoolID id = m_freeID;
    m_freeID = wrapper[id].next;

    return id;
}

template <typename T>
inline void ObjectPool<T>::deallocate(PoolID id)
{
    PoolSlot<T>* wrapper = (PoolSlot<T>*)m_pool->vector;
    wrapper[id].next = m_freeID;
    m_freeID = id;
}

template <typename T>
T* ObjectPool<T>::get(PoolID id)
{
    PoolSlot<T>* wrapper = (PoolSlot<T>*)m_pool->vector;

    return &wrapper[id].element;
}

template <typename T>
void ObjectPool<T>::realloc()
{
    PoolID lastID = m_capacity;

    if (m_capacity)
    {
        m_capacity *= 2;

        m_pool = array_resize(m_pool, m_capacity, sizeof(PoolSlot<T>));


        PoolSlot<T>* wrapper = (PoolSlot<T>*)m_pool->vector;

        memset(&wrapper[lastID], 0,
               (m_capacity - lastID) * sizeof(PoolSlot<T>));

    } else
    {
        m_capacity = 1;

        m_pool = array_create(m_capacity, sizeof(PoolSlot<T>));

        memset(m_pool->vector, 0, m_capacity * sizeof(PoolSlot<T>));
    }

    addFreeID(lastID);
}

template <typename T>
void ObjectPool<T>::addFreeID(PoolID first)
{
    PoolSlot<T>* wrapper = (PoolSlot<T>*)m_pool->vector;

    PoolID lastID = m_capacity - 1;
    for (PoolID id=first;id<lastID;id++)
        wrapper[id].next = id + 1;

    wrapper[lastID].next = m_freeID;
    m_freeID = first;
}



