#pragma once

#include "gr-base/externals/handle.h"

template<typename T>
class AssetPtr
{
public:
    Handle handle = HANDLE_INVALID;

    AssetPtr() = default;
    AssetPtr(Handle h) : handle(h) {}

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


