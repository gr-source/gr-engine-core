#include "gr-base/prefab/shaderprefab.hpp"
#include "gr-base/core/console.hpp"

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
        return it->second.get();
    return create(flags);
}

// private
gr::Shader* ShaderPrefab::create(ShaderPrefabFlags_ flags)
{
    std::string fragment = "#version 330 core\n";
    if ((m_asset.GetFlags() & ShaderAssetFlags_Line) != ShaderAssetFlags_Line)
    {
        if (flags & ShaderPrefabFlags_Instancing)
            fragment += "#define USE_INSTANCED\n";
    }

    fragment += m_asset.GetFragment();

    std::string vertex = "#version 330 core\n";
    if ((m_asset.GetFlags() & ShaderAssetFlags_Line) != ShaderAssetFlags_Line)
    {
        std::cout << "Is Line" << std::endl;
        if (flags & ShaderPrefabFlags_Instancing)
            vertex += "#define USE_INSTANCED\n";
    }

    vertex += m_asset.GetVertex();

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

        if (shader->build(&fragment_src, 1, &vertex_src, 1) == -1)
        {
            Console::LogError("[ AssetsManager::LoadShaderFromFile ] Shader.Build Fail to build shader, %s, %s", m_asset.GetName().c_str(), "null");
            return nullptr;
        }

        for (auto&& [name, info] : m_asset.GetUnifoms())
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

        m_shaders.push_back(shader);
    }

    return shader;
}

