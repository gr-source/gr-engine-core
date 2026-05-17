#include "gr-base/core/materialInstance.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

#include <cstring>

MaterialInstance::MaterialInstance(const Material* material) : m_material(material), m_shader(nullptr), isDirty(false)
{
    SetShader(material->GetShaderPrefab()->Get(ShaderPrefabFlags_none));
}

void MaterialInstance::sync()
{
    if (!isDirty)
        return;

    buildCache();

    isDirty = false;
}

void MaterialInstance::flush()
{
    if (m_gpuBuffer.empty())
        return;

    auto buffer = m_gpuBuffer.data();

    auto uniforms = m_shader->GetUniforms();
    for (UniformID id=0;id<m_shader->GetUniformCount();id++)
    {
        auto& uniform = uniforms[id];

        if (strstr(uniform.name, "u_mat_") != nullptr)
            continue;

        m_shader->SetUniform(id, buffer + uniform.offset);
    }
}

void MaterialInstance::buildCache()
{
    m_gpuBuffer.resize(m_shader->GetUniformBufferSize());

    std::cout << "Build: " << std::endl;

    for (size_t i=0;i<m_shader->GetUniformCount();i++)
    {
        auto& uniform = m_shader->GetUniforms()[i];

        const void* data = m_material->GetInternal(uniform.name);
        if (data != nullptr)
        {
            std::cout << uniform.name << std::endl;

            memcpy(m_gpuBuffer.data() + uniform.offset, data, uniform.stride);
        }
    }
}


