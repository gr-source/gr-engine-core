#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/core/console.hpp"

#include <algorithm>
#include <cstring>

static std::unordered_map<std::string, UniformType> mTypeMapping =
{
    {"float",       UniformType::FLOAT},
    {"int",         UniformType::INT},
    {"bool",        UniformType::BOOL},
    {"samplerCube", UniformType::SAMPLERCUBE},
    {"sampler2D",   UniformType::SAMPLER2D},
    {"mat3",        UniformType::MAT3},
    {"mat4",        UniformType::MAT4},
    {"vec2",        UniformType::VEC2},
    {"vec3",        UniformType::VEC3},
    {"vec4",        UniformType::VEC4},
};


ShaderPrefab::ShaderPrefab(const ShaderAsset& asset) : m_asset(asset)
{}

gr::Shader* ShaderPrefab::Get(ShaderPrefabFlags_ flags)
{
    auto it = m_shadervariant.find(flags);
    if (it != m_shadervariant.end())
        return it->second != nullptr ? it->second.get() : nullptr;
    return create(flags);
}

// private
gr::Shader* ShaderPrefab::create(ShaderPrefabFlags_ flags)
{
    std::string fragment = "#version 420\n";
    if ((m_asset.flags & ShaderAssetFlags_Line) != ShaderAssetFlags_Line)
    {
        if (flags & ShaderPrefabFlags_Instancing)
            fragment += "#define USE_INSTANCED\n";
    }

    fragment += m_asset.fragment;

    std::string vertex = "#version 420\n";
    if ((m_asset.flags & ShaderAssetFlags_Line) != ShaderAssetFlags_Line)
    {
        if (flags & ShaderPrefabFlags_Instancing)
            vertex += "#define USE_INSTANCED\n";
    }

    vertex += m_asset.vertex;

    const char* fragment_src = fragment.c_str();
    const char* vertex_src = vertex.c_str();

    auto&& [it, inserted] = m_shadervariant.emplace(flags, std::make_unique<gr::Shader>());

    gr::Shader* shader = it->second.get();
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
                Console::LogWarning("[ShaderLoadTask] fail in to registry: %s", name.c_str());
        }

        std::vector<gr::ShaderUniform> uniforms(shader->GetUniformCount());
        for (size_t i=0;i<shader->GetUniformCount();i++)
        {
            uniforms[i] = shader->GetUniforms()[i];
        }

        std::sort(uniforms.begin(), uniforms.end(),
        [](const gr::ShaderUniform& a, const gr::ShaderUniform& b)
        {
            bool aMat = strncmp(a.name, "u_mat_", 6) != 0;
            bool bMat = strncmp(b.name, "u_mat_", 6) != 0;

            if (aMat != bMat)
                return aMat;

            return strcmp(a.name, b.name) < 0;
        });

        void* dst = (void*)shader->GetUniforms();
        void* src = (void*)uniforms.data();

        // memcpy(dst, src, uniforms.size() * sizeof(gr::ShaderUniform));

        m_shaders.push_back(shader);
    }

    return shader;
}

