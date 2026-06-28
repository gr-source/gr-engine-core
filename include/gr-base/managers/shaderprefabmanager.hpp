#pragma once

#include "gr-base/asset/shaderasset.hpp"

#include "gr-base/externals/hash.hpp"
#include "gr-base/prefab/shaderprefab.hpp"

#include <memory>

class ShaderPrefabManager
{
public:
    ShaderPrefab* create(const ShaderAsset* src);

    ShaderPrefab* find(Hash64 hash) const;

    inline const std::vector<ShaderPrefab*>& GetAllShader3d() const
    {
        return m_shaders3d;
    }

private:
    std::unordered_map<Hash64, std::unique_ptr<ShaderPrefab>> m_prefabmap;

    std::vector<ShaderPrefab*> m_shaders3d;
};


