#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/externals/handle.h"
#include "gr-base/managers/default.hpp"

struct MeshFilter
{
    MeshHandle mesh = HANDLE_INVALID;
};


struct MeshRef
{
    AssetHash hash = INVALID_HASH64;
};


