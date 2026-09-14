#pragma once

#include <gr-render/shader.hpp>

enum class ShaderDomain
{
    PostProcess,
    Surface,
    Sprite,
    Debug
};

struct ShaderParams
{
    std::string name;
    std::string base;
    ShaderDomain domain;
};

typedef uint32_t ShaderStage;

enum ShaderStage_
{
    ShaderStage_Fragment = 1,
    ShaderStage_Vertex = 2,
    ShaderStage_Count = 3,
    ShaderStage_None = 4
};




