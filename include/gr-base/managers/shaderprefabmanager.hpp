#pragma once

#include "gr-base/asset/shaderasset.hpp"
#include "gr-base/asset/default.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

class ShaderPrefabManager
{
public:
    ShaderPrefab* create(AssetID id, const ShaderAsset* src);

    ShaderPrefab* find(AssetID id) const;

private:
    std::unordered_map<AssetID, std::unique_ptr<ShaderPrefab>> m_prefabmap;
};


