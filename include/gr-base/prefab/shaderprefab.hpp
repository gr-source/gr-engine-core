#pragma once

#include "gr-base/asset/shaderasset.hpp"

#include <gr-render/default.hpp>

#include <unordered_map>
#include <vector>
#include <memory>

typedef uint32_t ShaderVariantFlags;

enum ShaderVariantFlags_
{
    ShaderVariantFlags_None        = 0,
    ShaderVariantFlags_Instancing  = 1 << 0,
    ShaderVariantFlags_Skinning    = 1 << 1
};

class ShaderPrefab
{
public:
    ShaderPrefab(const ShaderAsset& asset);

    gr::shader* Get(ShaderVariantFlags flags);

private:
    std::unordered_map<ShaderVariantFlags, std::shared_ptr<gr::shader>> m_shadervariant;

    std::vector<gr::shader*> m_shaders;

    ShaderAsset m_asset;

    gr::shader* create(ShaderVariantFlags flags);
};



