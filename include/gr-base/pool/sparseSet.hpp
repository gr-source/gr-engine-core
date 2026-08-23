#pragma once

#include <type_traits>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <new>

#define INVALID_SPARSE_ID UINT32_MAX

typedef uint32_t SparseID;

template <typename T>
class SparseSet
{
public:
    SparseSet() :
        m_sparseCapacity(0),
        m_denseCount(0),
        m_denseCapacity(0),

        m_dense(nullptr),
        m_sparseToDense(nullptr),
        m_denseToSparse(nullptr)
    {}

    ~SparseSet()
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_t i=0;i<m_denseCount;i++)
            {
                T& data = m_dense[i];
                data.~T();
            }
        }

        if (m_dense != NULL)
            free(m_dense);

        if (m_sparseToDense != nullptr)
            free(m_sparseToDense);

        if (m_denseToSparse != nullptr)
            free(m_denseToSparse);
    }

    template <typename... Args>
    void insert(SparseID sparseID, Args&&... args)
    {
        while (sparseID >= m_sparseCapacity)
            grow_slot();

        uint32_t denseIndex = m_sparseToDense[sparseID];
        if (denseIndex == INVALID_SPARSE_ID)
        {
            if ((m_denseCount + 1) >= m_denseCapacity)
                grow_dense();

            denseIndex = static_cast<uint32_t>(m_denseCount++);

            m_sparseToDense[sparseID] = denseIndex;
            m_denseToSparse[denseIndex] = sparseID;
        } else
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                T& dst = m_dense[denseIndex];

                dst.~T();
            }
        }

        T* dst = m_dense + denseIndex;
        new (dst) T(std::forward<Args>(args)...);
    }

    void destroy(SparseID id)
    {
        T* ptr = get(id);
        if (ptr == nullptr)
            return;

        if constexpr (!std::is_trivially_destructible_v<T>)
            ptr->~T();

        deallocate(id);
    }

    inline T* get(SparseID sparseID) const
    {
        if (sparseID >= m_sparseCapacity)
            return nullptr;

        uint32_t denseID = m_sparseToDense[sparseID];
        if (denseID == INVALID_SPARSE_ID)
            return nullptr;

        return &m_dense[denseID];
    }

    inline size_t count() const
    {
        return m_denseCount;
    }

    inline T* data() const
    {
        return m_dense;
    }

private:
    size_t m_sparseCapacity = 0;

    size_t m_denseCapacity = 0;
    size_t m_denseCount = 0;
    T* m_dense = nullptr;

    uint32_t* m_sparseToDense = nullptr;
    uint32_t* m_denseToSparse = nullptr;

    void deallocate(SparseID sparseID)
    {
        SparseID removeDenseIndex = m_sparseToDense[sparseID];
        if (removeDenseIndex == INVALID_SPARSE_ID)
            return;

        SparseID lastDenseIndex = static_cast<SparseID>(--m_denseCount);
        if (removeDenseIndex != lastDenseIndex)
        {
            T* src = m_dense + lastDenseIndex;
            T* dst = m_dense + removeDenseIndex;

            if constexpr (!std::is_trivially_copyable_v<T>)
            {
                new (dst) T(std::move(*src));

                if constexpr (!std::is_trivially_destructible_v<T>)
                    src->~T();
            } else
            {
                memcpy(dst, src, sizeof(T));
            }

            m_denseToSparse[removeDenseIndex] = m_denseToSparse[lastDenseIndex];

            m_sparseToDense[m_denseToSparse[lastDenseIndex]] = removeDenseIndex;
        }

        m_sparseToDense[sparseID] = INVALID_SPARSE_ID;
    }

    void grow_slot()
    {
        size_t oldCapacity = m_sparseCapacity;
        m_sparseCapacity = m_sparseCapacity > 0 ? m_sparseCapacity * 2 : 2;

        m_sparseToDense = (SparseID*)realloc(m_sparseToDense, m_sparseCapacity * sizeof(SparseID));

        for (size_t i=oldCapacity;i<m_sparseCapacity;i++)
            m_sparseToDense[i] = INVALID_SPARSE_ID;
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

        m_denseToSparse = (SparseID*)realloc(m_denseToSparse, m_denseCapacity * sizeof(SparseID));
    }
};

template <typename T>
class SparseSet2
{
public:
    SparseSet2() :
        m_sparseCapacity(0),
        m_denseCount(0),
        m_denseCapacity(0),

        m_dense(nullptr),
        m_sparseToDense(nullptr),
        m_denseToSparse(nullptr)
    {}

    ~SparseSet2()
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            for (size_t i=0;i<m_denseCount;i++)
            {
                T& data = m_dense[i];
                data.~T();
            }
        }

        if (m_dense != NULL)
            free(m_dense);

        if (freeList != nullptr)
            free(freeList);

        if (m_sparseToDense != nullptr)
            free(m_sparseToDense);

        if (m_denseToSparse != nullptr)
            free(m_denseToSparse);
    }

    template <typename... Args>
    SparseID emplace(Args&&... args)
    {
        if (freeID == INVALID_SPARSE_ID)
            grow_slot();

        SparseID sparseID =
            freeID;

        freeID =
            freeList[sparseID];

        uint32_t denseIndex = m_sparseToDense[sparseID];
        if (denseIndex == INVALID_SPARSE_ID)
        {
            if ((m_denseCount + 1) >= m_denseCapacity)
                grow_dense();

            denseIndex = static_cast<uint32_t>(m_denseCount++);

            m_sparseToDense[sparseID] = denseIndex;
            m_denseToSparse[denseIndex] = sparseID;
        } else
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                T& dst = m_dense[denseIndex];

                dst.~T();
            }
        }

        T* dst = m_dense + denseIndex;
        new (dst) T(std::forward<Args>(args)...);

        return sparseID;
    }

    void destroy(SparseID id)
    {
        T* ptr = try_get(id);
        if (ptr == nullptr)
            return;

        if constexpr (!std::is_trivially_destructible_v<T>)
            ptr->~T();

        deallocate(id);
    }

    inline T* try_get(SparseID sparseID) const
    {
        if (sparseID >= m_sparseCapacity)
            return nullptr;

        uint32_t denseID = m_sparseToDense[sparseID];
        if (denseID == INVALID_SPARSE_ID)
            return nullptr;

        return &m_dense[denseID];
    }


    inline T& get(SparseID id) const
    {
        assert(id < m_sparseCapacity);

        const uint32_t denseID = m_sparseToDense[id];
        assert(denseID != INVALID_SPARSE_ID);

        return m_dense[denseID];
    }

    inline size_t count() const
    {
        return m_denseCount;
    }

    inline T* data() const
    {
        return m_dense;
    }

private:
    size_t m_sparseCapacity = 0;

    SparseID freeID =
        INVALID_SPARSE_ID;
    SparseID* freeList =
        nullptr;

    size_t m_denseCapacity = 0;
    size_t m_denseCount = 0;
    T* m_dense = nullptr;

    uint32_t* m_sparseToDense = nullptr;
    uint32_t* m_denseToSparse = nullptr;

    void deallocate(SparseID sparseID)
    {
        SparseID removeDenseIndex = m_sparseToDense[sparseID];
        if (removeDenseIndex == INVALID_SPARSE_ID)
            return;

        SparseID lastDenseIndex = static_cast<SparseID>(--m_denseCount);
        if (removeDenseIndex != lastDenseIndex)
        {
            T* src = m_dense + lastDenseIndex;
            T* dst = m_dense + removeDenseIndex;

            if constexpr (!std::is_trivially_copyable_v<T>)
            {
                new (dst) T(std::move(*src));

                if constexpr (!std::is_trivially_destructible_v<T>)
                    src->~T();
            } else
            {
                memcpy(dst, src, sizeof(T));
            }

            m_denseToSparse[removeDenseIndex] = m_denseToSparse[lastDenseIndex];

            m_sparseToDense[m_denseToSparse[lastDenseIndex]] = removeDenseIndex;
        }

        m_sparseToDense[sparseID] = INVALID_SPARSE_ID;

        freeList[sparseID] = freeID;
        freeID = sparseID;
    }

    void grow_slot()
    {
        size_t oldCapacity = m_sparseCapacity;
        m_sparseCapacity = m_sparseCapacity > 0 ? m_sparseCapacity * 2 : 2;

        freeList = (SparseID*)realloc(freeList, sizeof(SparseID) * m_sparseCapacity);
        m_sparseToDense = (SparseID*)realloc(m_sparseToDense, m_sparseCapacity * sizeof(SparseID));

        for (size_t i=oldCapacity;i<m_sparseCapacity;i++)
            m_sparseToDense[i] = INVALID_SPARSE_ID;

        addFreeList(oldCapacity);
    }

    void addFreeList(size_t first)
    {
        for (size_t i=first;i<m_sparseCapacity - 1;i++)
            freeList[i] = static_cast<SparseID>(i + 1);

        freeList[m_sparseCapacity - 1] = freeID;
        freeID = static_cast<SparseID>(first);
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

        m_denseToSparse = (SparseID*)realloc(m_denseToSparse, m_denseCapacity * sizeof(SparseID));
    }
};

