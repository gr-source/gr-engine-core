#include "gr-base/core/geometry/materialInstance.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include <cstring>

MaterialInstance::MaterialInstance(MaterialID materialID) : m_materialID(materialID), m_shader(nullptr)
{
    Material* material = AssetsManager::GetMaterial(m_materialID);

    m_shader = material->GetShader(material->bEnableInstancing);

    m_parametersBuffer.resize(m_shader->GetUniformBufferSize(), 0);
}

// public
void MaterialInstance::Flush() const
{
    gr::ShaderUniform* uniforms = m_shader->GetUniforms();

    size_t length = m_shader->GetUniformCount();
    for (size_t i=0;i<length;i++)
    {
        gr::ShaderUniform& uniform = uniforms[i];

        m_shader->SetUniform(uniform.id, m_parametersBuffer.data() + uniform.offset);
    }
}

// private
void MaterialInstance::Set(const char* name, const void* data, size_t size)
{
    UniformID uniformID = m_shader->findUniform(name);
    if (uniformID == GR_INVALID_ID)
        return;

    gr::ShaderUniform& shaderUniform = m_shader->GetUniforms()[uniformID];

    memcpy(m_parametersBuffer.data() + shaderUniform.offset, data, shaderUniform.stride);

    m_parameterMap[name] = uniformID;
}



