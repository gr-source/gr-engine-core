#pragma once

#include "gr-base/externals/hash.hpp"

#include <stdint.h>

#define INVALID_ASSET UINT32_MAX

typedef Hash64 AssetHash;

typedef uint32_t AssetID;
typedef uint32_t AssetFlags_;

enum AssetFlags
{
    AssetFlags_AssetModel     = 1 << 1,
    AssetFlags_AssetTexture   = 1 << 2,
    AssetFlags_AssetMesh      = 1 << 3,
    AssetFlags_AssetMaterial  = 1 << 4,
    AssetFlags_AssetShader    = 1 << 5
};

class MeshAsset;

