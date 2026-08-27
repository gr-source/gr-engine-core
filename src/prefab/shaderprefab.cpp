#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/core/console.hpp"

#include <gr-render/shader.hpp>
#include <gr-render/gl.h>
#include <iostream>

static std::unordered_map<std::string, gr::UniformType> mTypeMapping =
{
    {"float",       gr::UniformType::FLOAT},
    {"int",         gr::UniformType::INT},
    {"bool",        gr::UniformType::BOOL},
    {"samplerCube", gr::UniformType::SAMPLERCUBE},
    {"sampler2D",   gr::UniformType::SAMPLER2D},
    {"mat3",        gr::UniformType::MAT3},
    {"mat4",        gr::UniformType::MAT4},
    {"vec2",        gr::UniformType::VEC2},
    {"vec3",        gr::UniformType::VEC3},
    {"vec4",        gr::UniformType::VEC4},
};


ShaderPrefab::ShaderPrefab(const ShaderAsset& asset) : m_asset(asset)
{}

gr::shader* ShaderPrefab::Get(ShaderVariantFlags flags)
{
    auto it = m_shadervariant.find(flags);
    if (it != m_shadervariant.end())
        return it->second != nullptr ? it->second.get() : nullptr;
    return create(flags);
}

// private
gr::shader* ShaderPrefab::create(ShaderVariantFlags flags)
{
    std::string fragment = "#version 420\n";
    if (m_asset.domain == ShaderDomain::Surface)
    {
        if ((flags & ShaderVariantFlags_Instancing) != 0)
            fragment += "#define USE_INSTANCED\n";
        if ((flags & ShaderVariantFlags_BaseColorMap) != 0)
            fragment += "#define USE_BASECOLOR_MAP\n";
    }

    fragment += m_asset.fragment;

    std::string vertex = "#version 420\n";
    if (m_asset.domain == ShaderDomain::Surface)
    {
        if ((flags & ShaderVariantFlags_Instancing) != 0)
            vertex += "#define USE_INSTANCED\n";
        if ((flags & ShaderVariantFlags_BaseColorMap) != 0)
            vertex += "#define USE_BASECOLOR_MAP\n";
    }

    vertex += m_asset.vertex;

    const char* fragment_src = fragment.c_str();
    const char* vertex_src = vertex.c_str();

    auto shader = std::make_shared<gr::shader>();

    {
        std::string error;
        if (!shader->create(&fragment_src, 1, &vertex_src, 1, &error))
        {
            Console::LogError(
                "[Shader Build Fail] %s\n%s",
                m_asset.name.c_str(),
                error.c_str()
            );
            shader->destroy();

            m_shadervariant.erase(flags);

            return nullptr;
        }

        // m_shaders.push_back(shader);
    }

    m_shadervariant.emplace(flags, shader);

    m_shaders.push_back(shader.get());

    return shader.get();
}

