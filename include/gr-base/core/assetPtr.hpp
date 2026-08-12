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
    T* operator->();
};


