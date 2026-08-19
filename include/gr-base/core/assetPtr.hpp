#pragma once

#include <cstdint>

template<typename T>
class AssetPtr
{
public:
    uint32_t handle = UINT32_MAX;

    AssetPtr() = default;
    AssetPtr(uint32_t h) : handle(h) {}

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
};


