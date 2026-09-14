#include "gr-base/render/shader.hpp"

#include "gr-base/core/console.hpp"

#include <gr-render/shader.hpp>
#include <stdexcept>

Shader::Shader(const std::string& fragment, const std::string& vertex) :
    m_fragment(fragment),
    m_vertex(vertex)
{}

gr::shader& Shader::getProgram(ShaderFlags flags)
{
    if (m_flags == flags)
        return m_cache[flags];

    m_flags = flags;

    auto it =
        m_cache.find(flags);
    if (it != m_cache.end())
        return it->second;

    gr::shader& program =
        Build(flags);

    m_cache.emplace(flags, program);

    return program;
}

gr::shader& Shader::Build(ShaderFlags flags)
{
    std::string fragmentSrc = "#version 420\n";
    if ((flags & ShaderFlags_Instancing) != 0)
        fragmentSrc += "#define USE_INSTANCED\n";
    if ((flags & ShaderFlags_BaseColorMap) != 0)
        fragmentSrc += "#define USE_BASECOLOR_MAP\n";

    fragmentSrc += m_fragment;

    std::string vertexSrc = "#version 420\n";
    if ((flags & ShaderFlags_Instancing) != 0)
        vertexSrc += "#define USE_INSTANCED\n";
    if ((flags & ShaderFlags_BaseColorMap) != 0)
        vertexSrc += "#define USE_BASECOLOR_MAP\n";

    vertexSrc += m_vertex;

    const char* fragment_src = fragmentSrc.c_str();
    const char* vertex_src = vertexSrc.c_str();

    gr::shader shader;

    std::string error;
    if (!shader.create(&fragment_src, 1, &vertex_src, 1, &error))
        throw std::runtime_error(error);

    auto&& [it, added] =
        m_cache.emplace(flags, shader);

    return it->second;
}


