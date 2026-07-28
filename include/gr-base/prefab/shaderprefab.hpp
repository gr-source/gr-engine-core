#pragma once

#include "gr-base/asset/shaderasset.hpp"

#include <gr-render/default.hpp>

#include <unordered_map>
#include <vector>
#include <memory>

typedef uint8_t ShaderPrefabFlags_;

enum : uint8_t
{
    ShaderPrefabFlags_none        = 1 << 1,
    ShaderPrefabFlags_Instancing  = 1 << 2
};

class ShaderPrefab
{
public:
    ShaderPrefab(const ShaderAsset& asset);

    gr::shader* Get(ShaderPrefabFlags_ flags);

    inline const std::string& GetName() const
    {
        return m_asset.name;
    }

    inline const std::vector<gr::shader*>& GetShaders()
    {
        return m_shaders;
    }

    inline const ShaderAsset& GetAsset() const
    {
        return m_asset;
    }

private:
    std::unordered_map<ShaderPrefabFlags_, std::shared_ptr<gr::shader>> m_shadervariant;

    std::vector<gr::shader*> m_shaders;

    ShaderAsset m_asset;

    gr::shader* create(ShaderPrefabFlags_ flags);
};



