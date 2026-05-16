#include "gr-base/core/geometry/material.hpp"
#include "gr-base/managers/assetsManager.hpp"

#include <gr-render/shader.hpp>

#include <string.h>

Material::Material() : m_shader(nullptr), bEnableInstancing(false), m_shader_instanced(nullptr)
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

void Material::SetEnableInstancing()
{
    if (supportsInstancing())
        return;

    if (m_shader_instanced == nullptr)
    {
        m_shader_instanced = AssetsManager::FindShader(AssetsManager::GetShaderName(m_shader) + "_instanced");
        if (m_shader_instanced == nullptr)
            m_shader_instanced = AssetsManager::LoadShader2(AssetsManager::GetShaderName(m_shader) + "_instanced");
    }

    if (m_shader_instanced == nullptr)
        return;

    bEnableInstancing = true;
}

void Material::SetDisableInstancing()
{
    if (!supportsInstancing())
        return;

    bEnableInstancing = false;
}

bool Material::supportsInstancing() const
{
    return bEnableInstancing;
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

void Material::flush(gr::Shader* shader) const
{
    if (m_buffer.empty())
        return;

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






