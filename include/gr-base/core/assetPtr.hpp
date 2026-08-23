#pragma once

#include <cstdint>

template<typename T, typename IdType = uint32_t>
class AssetPtr
{
private:
    IdType id = UINT32_MAX;
public:
    AssetPtr() = default;
    AssetPtr(IdType id) : id(id) {}

    T* get();

    const T* get() const;

    T* operator->()
    {
        return get();
    }

    const T* operator->() const
    {
        return get();
    }

    T& operator*()
    {
        return *get();
    }

    const T& operator*() const
    {
        return *get();
    }

    explicit operator bool() const
    {
        return get() != nullptr;
    }

    explicit operator IdType() const
    {
        return id;
    }
};


