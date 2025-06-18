#include "gr-base/core/geometry/shader.hpp"

std::string GetShaderParamsName(ShaderParams shaderParams)
{
    int id = 0;

    if (shaderParams.flags & ShaderOptimize_static)
    {
        id |= ShaderOptimize_static;
    } else if (shaderParams.flags & ShaderOptimize_dynamic)
    {
        id |= ShaderOptimize_dynamic;
    }

    if (shaderParams.flags & ShaderOptimize_instanced)
    {
        id |= ShaderOptimize_instanced;
    }

    if (shaderParams.flags & ShaderOptimize_animated)
    {
        id |= ShaderOptimize_animated;
    }

    return shaderParams.name + '_' + std::to_string(static_cast<int>(shaderParams.type)) + '_' + std::to_string(id);
}


