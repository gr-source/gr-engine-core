#pragma once

#include <gr-render/gShader.h>

using ShaderOptimize_ = std::uint8_t;
using AssetShader_ = std::uint8_t;

enum AssetShader : AssetShader_
{
    AssetShader_None        = 0,
    AssetShader_Shader2D    = 1,
    AssetShader_Shader3D    = 2,
    AssetShader_Count
};

constexpr auto IAssetShaderCount = AssetShader_Count;

enum ShaderOptimize : ShaderOptimize_
{
    ShaderOptimize_None                 = 0,
    ShaderOptimize_DYNAMIC              = 1,
    ShaderOptimize_STATIC               = 2,
    ShaderOptimize_INSTANCED_DYNAMIC    = 3,
    ShaderOptimize_INSTANCED_STATIC     = 4,
    ShaderOptimize_ANIMATION_DYNAMIC    = 5,
    ShaderOptimize_ANIMATION_STATIC     = 6,
    ShaderOptimize_ANIMATION_INSTANCED  = 7,
    ShaderOptimize_Count
};

constexpr auto IShaderOptmize = ShaderOptimize_Count;

class Shader : public grr::gShader
{
public:
    Shader(AssetShader_ type, ShaderOptimize_ optimize);

    const AssetShader_ type;

    const ShaderOptimize_ optimize;
};



