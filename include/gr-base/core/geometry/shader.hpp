#pragma once

#include <gr-render/gShader.h>

using ShaderOptimize_ = std::uint8_t;

enum class ShaderType
{
    SHADER2D,
    SHADER3D,
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
    ShaderType type;
    ShaderOptimize_ flags;
};

std::string GetShaderParamsName(ShaderParams shaderParams);

ShaderParams GetShaderParamsFromName(const std::string &name);




