#pragma once

#include "gr-base/asset/shaderasset.hpp"
#include <gr-render/shader.hpp>

#include <unordered_map>
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

    gr::Shader* Get(ShaderPrefabFlags_ flags);

    inline const std::string& GetName() const
    {
        return m_asset.GetName();
    }

    inline const std::vector<gr::Shader*>& GetShaders()
    {
        return m_shaders;
    }

private:
    std::unordered_map<ShaderPrefabFlags_, std::unique_ptr<gr::Shader>> m_shadervariant;

    std::vector<gr::Shader*> m_shaders;

    ShaderAsset m_asset;

    gr::Shader* create(ShaderPrefabFlags_ flags);
};



