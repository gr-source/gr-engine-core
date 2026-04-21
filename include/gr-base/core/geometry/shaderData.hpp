#pragma once

#include <gr-render/shader.hpp>

using ShaderOptimize_ = std::uint8_t;

enum class ShaderType : uint8_t
{
    NONE,
    SHADER2D,
    SHADER3D,
    SKY,
    LINE,
    COUNT
};

constexpr uint8_t ShaderTypeCount = static_cast<uint8_t>(ShaderType::COUNT);

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



