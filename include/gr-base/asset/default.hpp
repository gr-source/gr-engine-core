#pragma once

#include "gr-base/asset/iasset.hpp"

#include "gr-base/externals/hash.hpp"

#include "gr-base/core/ievent.hpp"

#include <stdint.h>

#define INVALID_ASSET UINT32_MAX

typedef Hash64 AssetHash;

typedef uint32_t AssetID;

enum AssetType
{
    AssetType_Unknown   = 0,
    AssetType_Model     = 1,
    AssetType_Texture   = 2,
    AssetType_Mesh      = 3,
    AssetType_Material  = 4,
    AssetType_Shader    = 5
};

class MeshAsset;

struct AssetEventOnLoaded : public IEvent
{
};

struct AssetEventOnCreated : public IEvent
{
    AssetID value = INVALID_ASSET;
};

struct AssetEventOnRemove : public IEvent
{
    AssetID value = INVALID_ASSET;
};

