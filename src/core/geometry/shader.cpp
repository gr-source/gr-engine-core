#include "gr-base/core/geometry/shader.hpp"

constexpr char KEY = '\x1F';

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

    return shaderParams.name + KEY + shaderParams.base + KEY + std::to_string(static_cast<int>(shaderParams.type)) + KEY + std::to_string(id);
}


ShaderParams GetShaderParamsFromName(const std::string &name)
{
    ShaderParams shaderParams;
    size_t firstUnderscore = name.find(KEY);
    size_t secondUnderscore = name.find(KEY, firstUnderscore + 1);
    size_t thirtUnderscore = name.find(KEY, secondUnderscore + 1);

    if (firstUnderscore == std::string::npos || secondUnderscore == std::string::npos || thirtUnderscore == std::string::npos)
    {
        return shaderParams;
    }

    shaderParams.name = name.substr(0, firstUnderscore);
    shaderParams.base = name.substr(firstUnderscore + 1, secondUnderscore - firstUnderscore - 1);

    shaderParams.type = static_cast<ShaderType>(std::stoi(name.substr(secondUnderscore + 1, thirtUnderscore - secondUnderscore - 1)));

    int id = std::stoi(name.substr(thirtUnderscore + 1));

    shaderParams.flags = 0;
    if (id & ShaderOptimize_static) {
        shaderParams.flags |= ShaderOptimize_static;
    }
    if (id & ShaderOptimize_dynamic) {
        shaderParams.flags |= ShaderOptimize_dynamic;
    }
    if (id & ShaderOptimize_instanced) {
        shaderParams.flags |= ShaderOptimize_instanced;
    }
    if (id & ShaderOptimize_animated) {
        shaderParams.flags |= ShaderOptimize_animated;
    }

    return shaderParams;
}


