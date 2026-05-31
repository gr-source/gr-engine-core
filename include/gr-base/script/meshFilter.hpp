#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/managers/default.hpp"

struct MeshFilter
{
    MeshHandle mesh;
};


struct MeshRef
{
    AssetHash hash = INVALID_HASH64;
};


