#pragma once

#include "gr-base/core/geometry/shaderData.hpp"

#include "gr-base/asset/iasset.hpp"

#include <unordered_map>
#include <cstdint>
#include <string>

struct ShaderAsset : IAsset
{
    std::string fragment;

    std::string vertex;

    std::string name;

    ShaderDomain domain;

    bool serialize(Stream* stream) const override;

    bool deserialize(Stream* stream) override;
};

