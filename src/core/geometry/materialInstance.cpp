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

void MaterialInstance::Serialize(Stream* stream) const
{
    uint64_t count = static_cast<uint64_t>(m_parameterMap.size());
    if (!stream->write(count))
        return;

    auto uniforms = m_shader->GetUniforms();
    auto buffer = m_parametersBuffer.data();

    for (auto &&[name, uniformID] : m_parameterMap)
    {
        if (!stream->write(name))
            return;

        auto& uniform = uniforms[uniformID];
        if (!stream->write(uniform.stride))
            return;

        if (!stream->write(buffer + uniform.offset, uniform.stride))
            return;
    }
}

void MaterialInstance::Deserialize(Stream* stream)
{
    uint64_t count = 0;
    if (!stream->read(count))
        return;

    for (size_t i=0;i<count;i++)
    {
        std::string name;
        if (!stream->read(name))
            return;

        uint32_t stride = 0;
        if (!stream->read(stride))
            return;

        std::vector<uint8_t> data(stride, 0);
        if (!stream->read(data.data(), stride))
            return;

        Set(name.c_str(), data.data(), data.size());
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



