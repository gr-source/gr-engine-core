#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/core/console.hpp"

#include <gr-render/shader.hpp>
#include <gr-render/gl.h>

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

