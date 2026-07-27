#pragma once

#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/externals/hash.hpp"
#include "gr-base/prefab/shaderprefab.hpp"

#include <memory>

class ShaderPrefabManager
{
public:
    ShaderPrefab* create(AssetID id, const ShaderAsset* src);

    ShaderPrefab* find(AssetID id) const;

private:
    std::unordered_map<AssetID, std::unique_ptr<ShaderPrefab>> m_prefabmap;
};


