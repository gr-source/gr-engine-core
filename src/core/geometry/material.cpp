#include "gr-base/core/geometry/material.hpp"

#include <gr-render/shader.hpp>

#include <string.h>

Material::Material() : m_shader(nullptr), m_shaderInstancing(nullptr), bEnableInstancing(false)
{
    m_textures.fill(nullptr);
}

Material::~Material()
{}

void Material::setTexture(int idx, gr::gTexture *texture)
{
    m_textures[idx >= m_textures.size() ? 0 : idx] = texture;
}

gr::gTexture *Material::getTexture(int idx) const
{
    return m_textures[idx >= m_textures.size() ? 0 : idx];
}

std::array<gr::gTexture *, 10> Material::getTextures() const
{
    return m_textures;
}

gr::Shader *Material::GetShader(bool instancing) const
{
    return instancing ? m_shaderInstancing : m_shader;
}

void Material::SetShader(gr::Shader *shader, bool instancing)
{
    if (instancing)
        m_shaderInstancing = shader;
    else
        m_shader = shader;
}

bool Material::supportsInstancing() const
{
    return m_shaderInstancing != nullptr;
}

void Material::flush(gr::Shader* shader) const
{
    auto buffer = m_buffer.data();

    auto uniforms = shader->GetUniforms();
    for (UniformID id=0;id<shader->GetUniformCount();id++)
    {
        auto& uniform = uniforms[id];

        if (strstr(uniform.name, "u_mat_") != nullptr)
            continue;

        shader->SetUniform(id, buffer + uniform.offset);
    }
}

bool Material::setupTexture(int id, gr::gTexture *texture)
{
    m_textures[id > 10 ? 0 : id] = texture;
    return true;
}

void Material::Set(const char *name, u8 count, const void *data)
{
    UniformID id = m_shader->findUniform(name);
    if (id == GR_INVALID_ID)
        return;

    auto &uniform = m_shader->GetUniforms()[id];
    m_buffer.resize(m_shader->GetUniformBufferSize(), 0);

    memcpy(m_buffer.data() + uniform.offset, data, uniform.stride);
}

void* Material::Get(const char* name)
{
    if (m_buffer.empty())
        return nullptr;

    UniformID id = m_shader->findUniform(name);
    if (id == GR_INVALID_ID)
        return nullptr;

    auto &uniform = m_shader->GetUniforms()[id];
    return m_buffer.data() + uniform.offset;
}






