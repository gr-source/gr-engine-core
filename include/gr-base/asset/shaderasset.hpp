#pragma once

#include "default.hpp"
#include "gr-base/asset/iasset.hpp"

#include <unordered_map>
#include <cstdint>
#include <string>

typedef uint32_t ShaderAssetFlags_;

enum : uint32_t
{
    ShaderAssetFlags_Shader3D   = 1 << 1,
    ShaderAssetFlags_Shader2D   = 1 << 2,
    ShaderAssetFlags_Line       = 1 << 3
};

struct ShaderAsset : IAsset
{
    std::unordered_map<std::string, std::tuple<std::string, int>> parameters;

    std::string fragment;

    std::string vertex;

    std::string name;

    uint32_t flags;

    void AddUniform(const std::string& name, const std::string& typesname, int count);

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;
};

