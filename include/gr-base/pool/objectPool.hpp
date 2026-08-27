#pragma once

#include <cstdlib>
#include <utility>

#include <stdint.h>
#include <string.h>

#include "gr-base/externals/handle.h"

#define INVALID_ID (PoolID)-1

typedef uint32_t PoolID;

template <typename T>
class ObjectPool
{
public:
    ObjectPool() :
        m_slotCapacity(0),
        m_denseCount(0),
        m_denseCapacity(0),
        m_dense(nullptr),
        m_generations(nullptr),
        m_freeList(nullptr),
        m_freeID(INVALID_ID),
        m_sparseToDense(nullptr),
        m_denseToSparse(nullptr)
    {}

    ~ObjectPool()
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_t i=0;i<m_denseCount;i++)
            {
                T& data = m_dense[i];
                data.~T();
            }
        }

        if (m_dense != nullptr)
            free(m_dense);

        if (m_generations != nullptr)
            free(m_generations);

        if (m_freeList != nullptr)
            free(m_freeList);

        if (m_sparseToDense != nullptr)
            free(m_sparseToDense);

        if (m_denseToSparse != nullptr)
            free(m_denseToSparse);
    }

    ObjectPool(const ObjectPool& other) noexcept :
        m_slotCapacity(other.m_slotCapacity),
        m_denseCount(other.m_denseCount),
        m_denseCapacity(other.m_denseCapacity),
        m_dense(nullptr),
        m_generations(nullptr),
        m_freeList(nullptr),
        m_freeID(other.m_freeID),
        m_sparseToDense(nullptr),
        m_denseToSparse(nullptr)
    {
        if (other.m_sparseToDense != nullptr)
        {
            m_sparseToDense =
                (PoolID*)malloc(m_slotCapacity * sizeof(PoolID));

            memcpy(m_sparseToDense, other.m_sparseToDense, m_slotCapacity * sizeof(PoolID));
        }

        if (other.m_generations != nullptr)
        {
            m_generations =
                (PoolID*)malloc(m_slotCapacity * sizeof(uint32_t));

            memcpy(m_generations, other.m_generations, m_slotCapacity * sizeof(uint32_t));
        }

        if (other.m_freeList != nullptr)
        {
            m_freeList =
                (PoolID*)malloc(m_slotCapacity * sizeof(PoolID));

            memcpy(m_freeList, other.m_freeList, m_slotCapacity * sizeof(PoolID));
        }

        if (other.m_denseToSparse != nullptr)
        {
            m_denseToSparse =
                (PoolID*)malloc(m_denseCapacity * sizeof(PoolID));

            memcpy(m_denseToSparse, other.m_denseToSparse, m_denseCapacity * sizeof(PoolID));
        }

        if (other.m_dense != nullptr)
        {
            if constexpr (!std::is_trivially_copyable_v<T>)
            {
                T* array = (T*)malloc(m_denseCapacity * sizeof(T));
                for (size_t i=0;i<m_denseCount;i++)
                {
                    T* src = other.m_dense + i;
                    T* dst = array + i;

                    new (dst) T(*src);
                }

                m_dense = array;
            } else
            {
                m_dense =
                    (T*)malloc(m_dense, m_denseCapacity * sizeof(T));

                memcpy(m_dense, other.m_dense, m_denseCapacity * sizeof(T));
            }
        }
    }

    ObjectPool(ObjectPool&& other) noexcept :
        m_slotCapacity(other.m_slotCapacity),
        m_denseCount(other.m_denseCount),
        m_denseCapacity(other.m_denseCapacity),
        m_dense(other.m_dense),
        m_generations(other.m_generations),
        m_freeList(other.m_freeList),
        m_freeID(other.m_freeID),
        m_sparseToDense(other.m_sparseToDense),
        m_denseToSparse(other.m_denseToSparse)
    {
        other.m_slotCapacity = 0;

        other.m_generations = nullptr;

        other.m_denseCapacity = 0;
        other.m_denseCount = 0;
        other.m_dense = nullptr;

        other.m_freeList = nullptr;
        other.m_freeID = INVALID_ID;

        other.m_sparseToDense = nullptr;
        other.m_denseToSparse = nullptr;
    }

    ObjectPool& operator=(const ObjectPool& other)
    {
        if (this != &other)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                for (size_t i=0;i<m_denseCount;i++)
                {
                    T& data = m_dense[i];
                    data.~T();
                }
            }

            if (m_dense != nullptr)
                free(m_dense);

            if (m_generations != nullptr)
                free(m_generations);

            if (m_freeList != nullptr)
                free(m_freeList);

            if (m_sparseToDense != nullptr)
                free(m_sparseToDense);

            if (m_denseToSparse != nullptr)
                free(m_denseToSparse);

            m_slotCapacity = other.m_slotCapacity;
            m_denseCapacity = other.m_denseCapacity;
            m_denseCount = other.m_denseCount;
            m_freeID = other.m_freeID;

            m_sparseToDense = nullptr;
            m_denseToSparse = nullptr;
            m_generations = nullptr;
            m_freeList = nullptr;
            m_dense = nullptr;

            if (other.m_sparseToDense != nullptr)
            {
                m_sparseToDense =
                    (PoolID*)malloc(m_slotCapacity * sizeof(PoolID));

                memcpy(m_sparseToDense, other.m_sparseToDense, m_slotCapacity * sizeof(PoolID));
            }

            if (other.m_generations != nullptr)
            {
                m_generations =
                    (PoolID*)malloc(m_slotCapacity * sizeof(uint32_t));

                memcpy(m_generations, other.m_generations, m_slotCapacity * sizeof(uint32_t));
            }

            if (other.m_freeList != nullptr)
            {
                m_freeList =
                    (PoolID*)malloc(m_slotCapacity * sizeof(PoolID));

                memcpy(m_freeList, other.m_freeList, m_slotCapacity * sizeof(PoolID));
            }

            if (other.m_denseToSparse != nullptr)
            {
                m_denseToSparse =
                    (PoolID*)malloc(m_denseCapacity * sizeof(PoolID));

                memcpy(m_denseToSparse, other.m_denseToSparse, m_denseCapacity * sizeof(PoolID));
            }

            if (other.m_dense != nullptr)
            {
                if constexpr (!std::is_trivially_copyable_v<T>)
                {
                    T* array = (T*)malloc(m_denseCapacity * sizeof(T));
                    for (size_t i=0;i<m_denseCount;i++)
                    {
                        T* src = other.m_dense + i;
                        T* dst = array + i;

                        new (dst) T(*src);
                    }

                    m_dense = array;
                } else
                {
                    m_dense =
                        (T*)malloc(m_dense, m_denseCapacity * sizeof(T));

                    memcpy(m_dense, other.m_dense, m_denseCapacity * sizeof(T));
                }
            }
        }
        return *this;
    }

    Handle push(T&& value)
    {
        return emplace(std::move(value));
    }

    template <typename... Args>
    Handle emplace(Args&&... args)
    {
        Handle h = allocate();

        T* ptr = get(h);
        new (ptr) T(std::forward<Args>(args)...);

        return h;
    }

    void destroy(Handle h)
    {
        T* element = get(h);
        if (element == nullptr)
            return;

        if constexpr (!std::is_trivially_destructible_v<T>)
            element->~T();

        deallocate(h);
    }

    inline T* get(Handle h) const
    {
        uint32_t sparseID = handle_get_first(h);
        uint32_t gen = handle_get_second(h);

        if (sparseID >= m_slotCapacity)
            return nullptr;

        if (gen != m_generations[sparseID])
            return nullptr;

        uint32_t denseID = m_sparseToDense[sparseID];
        if (denseID == INVALID_ID)
            return nullptr;

        return &m_dense[denseID];
    }

    inline T* data() const
    {
        return m_dense;
    }

    inline size_t count() const
    {
        return m_denseCount;
    }

private:
    struct Slot
    {
        uint32_t index;
        uint32_t gen;
    };

    size_t m_slotCapacity = 0;

    uint32_t* m_generations = nullptr;

    size_t m_denseCapacity = 0;
    size_t m_denseCount = 0;
    T* m_dense = nullptr;

    PoolID* m_freeList = nullptr;
    PoolID m_freeID = INVALID_ID;

    uint32_t* m_sparseToDense = nullptr;
    uint32_t* m_denseToSparse = nullptr;

    Handle allocate()
    {
        if (m_freeID == INVALID_ID)
            grow_slot();

        uint32_t slotID = m_freeID;
        m_freeID = m_freeList[slotID];

        if ((m_denseCount + 1) >= m_denseCapacity)
            grow_dense();

        uint32_t index = static_cast<uint32_t>(m_denseCount++);

        m_sparseToDense[slotID] = index;
        m_denseToSparse[index] = slotID;

        uint32_t gen = m_generations[slotID];

        return handle_create(slotID, gen);
    }

    void deallocate(Handle h)
    {
        uint32_t slotID = handle_get_first(h);
        uint32_t gen = handle_get_second(h);
        if (gen != m_generations[slotID])
            return;

        PoolID id = m_sparseToDense[slotID];

        PoolID last = static_cast<PoolID>(--m_denseCount);
        if (id != last)
        {
            T* src = m_dense + last;
            T* dst = m_dense + id;

            if constexpr (!std::is_trivially_copyable_v<T>)
            {
                new (dst) T(std::move(*src));

                if constexpr (!std::is_trivially_destructible_v<T>)
                    src->~T();
            } else
            {
                memcpy(dst, src, sizeof(T));
            }

            m_denseToSparse[id] = m_denseToSparse[last];

            m_sparseToDense[m_denseToSparse[last]] = id;
        }

        m_sparseToDense[slotID] = INVALID_ID;

        m_generations[slotID]++;

        m_freeList[slotID] = m_freeID;
        m_freeID = slotID;
    }

    void grow_slot()
    {
        size_t oldCapacity = m_slotCapacity;
        m_slotCapacity = m_slotCapacity > 0 ? m_slotCapacity * 2 : 2;

        m_sparseToDense = (PoolID*)realloc(m_sparseToDense, m_slotCapacity * sizeof(PoolID));
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

        if constexpr (!std::is_trivially_copyable_v<T>)
        {
            T* array = (T*)malloc(m_denseCapacity * sizeof(T));
            if (m_dense != nullptr)
            {
                for (size_t i=0;i<m_denseCount;i++)
                {
                    T* src = m_dense + i;
                    T* dst = array + i;

                    new (dst) T(std::move(*src));

                    if constexpr (!std::is_trivially_destructible_v<T>)
                        src->~T();
                }

                free(m_dense);
            }
            m_dense = array;
        } else
        {
            m_dense = (T*)realloc(m_dense, m_denseCapacity * sizeof(T));
        }

        m_denseToSparse = (PoolID*)realloc(m_denseToSparse, m_denseCapacity * sizeof(PoolID));
    }

    void addFreeList(size_t oldCapacity)
    {
        for (size_t i=oldCapacity;i<m_slotCapacity - 1;i++)
            m_freeList[i] = static_cast<PoolID>(i + 1);

        m_freeList[m_slotCapacity - 1] = m_freeID;
        m_freeID = static_cast<PoolID>(oldCapacity);
    }
};
