#pragma once

#include "asset.hpp"
#include <cstdint>
#include <unordered_map>

typedef uint32_t ShaderAssetFlags_;

enum : uint32_t
{
    ShaderAssetFlags_Shader3D   = 1 << 1,
    ShaderAssetFlags_Shader2D   = 1 << 2,
    ShaderAssetFlags_Line       = 1 << 3
};

struct ShaderAsset
{
    std::unordered_map<std::string, std::tuple<std::string, int>> parameters;

    std::string fragment;

    std::string vertex;

    std::string name;

    uint32_t flags;

    void AddUniform(const std::string& name, const std::string& typesname, int count);

    bool serialize(Stream* stream) const;

    bool deserialize(Stream* stream);
};

