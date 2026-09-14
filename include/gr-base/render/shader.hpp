#pragma once

#include <gr-render/default.hpp>

#include <unordered_map>
#include <cstdint>
#include <string>

typedef uint32_t ShaderFlags;

enum ShaderFlags_
{
    ShaderFlags_None            = 0,
    ShaderFlags_Instancing      = 1 << 0,
    ShaderFlags_Static          = 1 << 1,
    ShaderFlags_BaseColorMap    = 1 << 2
};

class Shader
{
public:
    Shader(const std::string& fragment, const std::string& vertex);

    gr::shader& getProgram(ShaderFlags flags);

private:
    std::unordered_map<ShaderFlags, gr::shader> m_cache;

    const std::string m_fragment;

    const std::string m_vertex;

    ShaderFlags m_flags =
        UINT32_MAX;

    gr::shader& Build(ShaderFlags flags);
};

