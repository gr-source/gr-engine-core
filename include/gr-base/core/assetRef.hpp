#pragma once

#include "gr-base/externals/handle.h"

template<typename T>
class AssetPtr
{
public:
    AssetPtr() = delete;
    AssetPtr(Handle h) : handle(h) {}

    T* Get();
    T* operator->();

private:
    Handle handle = HANDLE_INVALID;
};


