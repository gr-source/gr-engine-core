#pragma once

#include <cstdlib>
#include <utility>

#include <stdint.h>

#define INVALID_SPARSE_ID UINT32_MAX

typedef uint32_t SparseID;

template <typename T>
class SparseSet
{
public:
    SparseSet() :
        m_slotCapacity(0),
        m_denseCount(0),
        m_denseCapacity(0),

        m_dense(nullptr),
        m_sparse(nullptr),
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

        if (m_sparse != nullptr)
            free(m_sparse);

        if (m_sparseToDense != nullptr)
            free(m_sparseToDense);

        if (m_denseToSparse != nullptr)
            free(m_denseToSparse);
    }

    void insert(SparseID sparseID, const T* src = nullptr)
    {
        while (sparseID >= m_slotCapacity)
            grow_slot();

        uint32_t denseIndex = m_sparseToDense[sparseID];
        if (denseIndex == INVALID_SPARSE_ID)
        {
            denseIndex = static_cast<uint32_t>(m_denseCount++);
            if (denseIndex >= m_denseCapacity)
                grow_dense();

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

        if (src != nullptr)
            new (dst) T(std::move(*src));
        else
            new (dst) T();
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
        if (sparseID >= m_slotCapacity)
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
    struct Slot
    {
        uint32_t index;
        uint32_t gen;
    };

    size_t m_slotCapacity;

    SparseID* m_sparse;

    size_t m_denseCapacity;
    size_t m_denseCount;
    T* m_dense;

    uint32_t* m_sparseToDense;
    uint32_t* m_denseToSparse;

    void deallocate(SparseID sparseID)
    {
        SparseID removeDenseIndex = m_sparseToDense[sparseID];

        SparseID lastDenseIndex = static_cast<SparseID>(--m_denseCount);
        if (removeDenseIndex != lastDenseIndex)
        {
            T* src = m_dense + lastDenseIndex;
            T* dst = m_dense + removeDenseIndex;

            new (dst) T(std::move(*src));

            m_denseToSparse[removeDenseIndex] = m_denseToSparse[lastDenseIndex];

            m_sparseToDense[m_denseToSparse[lastDenseIndex]] = removeDenseIndex;
        }

        m_sparseToDense[sparseID] = INVALID_SPARSE_ID;
    }

    void grow_slot()
    {
        size_t oldCapacity = m_slotCapacity;
        m_slotCapacity = m_slotCapacity > 0 ? m_slotCapacity * 2 : 2;

        m_sparseToDense = (SparseID*)realloc(m_sparseToDense, m_slotCapacity * sizeof(SparseID));

        for (size_t i=oldCapacity;i<m_slotCapacity;i++)
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


