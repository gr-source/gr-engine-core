#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/core/console.hpp"

#include <gr-render/shader.hpp>

#include <algorithm>
#include <cstring>

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
    if ((m_asset.flags & ShaderAssetFlags_Line) != ShaderAssetFlags_Line)
    {
        if ((flags & ShaderVariantFlags_Instancing) != 0)
            fragment += "#define USE_INSTANCED\n";
    }

    fragment += m_asset.fragment;

    std::string vertex = "#version 420\n";
    if ((m_asset.flags & ShaderAssetFlags_Line) != ShaderAssetFlags_Line)
    {
        if ((flags & ShaderVariantFlags_Instancing) != 0)
            vertex += "#define USE_INSTANCED\n";
    }

    vertex += m_asset.vertex;

    const char* fragment_src = fragment.c_str();
    const char* vertex_src = vertex.c_str();

    auto&& [it, inserted] = m_shadervariant.emplace(flags, std::make_shared<gr::shader>());

    gr::shader* shader = it->second.get();
    if (inserted)
    {
        // std::cout << "[ ===== " << m_asset.GetName() << " ===== ]" << std::endl;
        // std::cout << "============================ fragment ============================" << std::endl;
        // std::cout << fragment_src << std::endl;
        // std::cout << "============================ vertex ============================" << std::endl;
        // std::cout << vertex_src << std::endl;

        std::string error;
        if (!shader->create(&fragment_src, 1, &vertex_src, 1, &error))
        {
            Console::LogError(
                "[Shader Build Fail] %s\n%s",
                m_asset.name.c_str(),
                error.c_str()
            );

            m_shadervariant.erase(flags);

            return nullptr;
        }

        for (auto&& [name, info] : m_asset.parameters)
        {
            auto &&[typesname, count] = info;

            auto it = mTypeMapping.find(typesname);
            if (it == mTypeMapping.end())
            {
                Console::LogWarning("[ ShaderLoadTask ] Vertex Shader uniform type do not is valid: %s, %s", name.c_str(), typesname.c_str());
                continue;
            }

            if (shader->registry(name.c_str(), count, it->second) == GR_INVALID_ID)
                Console::LogWarning("[ ShaderLoadTask ] uniform '%s' not found!", name.c_str());
        }

        std::vector<gr::uniform> uniforms(shader->count);
        for (size_t i=0;i<shader->count;i++)
        {
            uniforms[i] = shader->uniforms[i];
        }

        std::sort(uniforms.begin(), uniforms.end(),
        [](const gr::uniform& a, const gr::uniform& b)
        {
            bool aMat = strncmp(a.name, "u_mat_", 6) != 0;
            bool bMat = strncmp(b.name, "u_mat_", 6) != 0;

            if (aMat != bMat)
                return aMat;

            return strcmp(a.name, b.name) < 0;
        });

        void* dst = (void*)shader->uniforms;
        void* src = (void*)uniforms.data();

        // memcpy(dst, src, uniforms.size() * sizeof(gr::uniform));

        m_shaders.push_back(shader);
    }

    return shader;
}

