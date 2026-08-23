#pragma once

#include <gr-render/shader.hpp>

using ShaderOptimize_ = std::uint8_t;

enum class ShaderDomain
{
    PostProcess,
    Surface,
    Sprite,
    Debug
};

enum ShaderOptimize : ShaderOptimize_
{
    ShaderOptimize_static       = 1 << 0,
    ShaderOptimize_dynamic      = 1 << 1,
    ShaderOptimize_animated     = 1 << 2,
    ShaderOptimize_instanced    = 1 << 3,
};

struct ShaderParams
{
    std::string name;
    std::string base;
    ShaderDomain domain;
    ShaderOptimize_ flags;
};


enum class ShaderBuildType
{
    NONE,
    FRAGMENT,
    VERTEX,
    COUNT
};

#define IShaderStage 3

std::string GetShaderParamsName(ShaderParams shaderParams);

ShaderParams GetShaderParamsFromName(const std::string &name);



