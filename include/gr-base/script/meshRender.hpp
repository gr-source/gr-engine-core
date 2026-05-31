#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/managers/default.hpp"

struct MeshRender
{
    MaterialHandle* materials = nullptr;
    size_t numMaterials = 0;
};

struct MaterialRef
{
    AssetHash* materials = nullptr;
    size_t numMaterials = 0;
};




