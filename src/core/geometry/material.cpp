#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/shader.hpp"

#include <gr-render/gShader.h>

#include <string.h>

Material::Material() : m_shader(nullptr), m_shaderInstancing(nullptr), m_scalling{1.0f, 1.0f}, m_offset{0.0f, 0.0f}, bEnableInstancing(false)
{
    m_textures.fill(nullptr);
}

Material::~Material()
{
    /*
    for (auto &&[name, ptr] : m_uniformMap)
    {
        if (ptr != nullptr)
        {
            std::free(ptr);
        }
    }
    */
}

void Material::setTexture(int idx, grr::gTexture *texture)
{
    m_textures[idx >= m_textures.size() ? 0 : idx] = texture;
}

grr::gTexture *Material::getTexture(int idx) const
{
    return m_textures[idx >= m_textures.size() ? 0 : idx];
}

std::array<grr::gTexture *, 10> Material::getTextures() const
{
    return m_textures;
}

grr::gShader *Material::GetShader(bool instancing) const
{
    return instancing ? m_shaderInstancing : m_shader;
}

void Material::SetShader(grr::gShader *shader, bool instancing)
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

void Material::set_scaling(const Vector2 &value)
{
    m_scalling = value;
}

Vector2 &Material::get_scalling()
{
    return m_scalling;
}

void Material::set_offset(const Vector2 &value)
{
    m_offset = value;
}

Vector2 &Material::get_offset()
{
    return m_offset;
}

void Material::serialize(Stream *stream) const
{
    {
        auto name = AssetsManager::GetShaderName(m_shader);
        if (!stream->write(name))
            return;
    }

    bool isInstancing = supportsInstancing();
    if (!stream->write(isInstancing))
        return;

    if (isInstancing)
    {
        auto name = AssetsManager::GetShaderName(m_shaderInstancing);
        if (!stream->write(name))
            return;
    }

    std::vector<std::tuple<grr::gTexture *, int>> textures;
    for (std::size_t i={};i<m_textures.size();i++)
    {
        if (m_textures[i] != nullptr)
            textures.emplace_back(m_textures[i], i);
    }

    auto tCount = textures.size();
    if (!stream->write(&tCount, sizeof(std::size_t)))
    {
        return;
    }

    for (auto &&[texture, id] : textures)
    {
        if (!stream->write(reinterpret_cast<const char *>(&id), sizeof(int)))
        {
            return;
        }

        std::string path;
        try
        {
            path = AssetsManager::GetRefTextureName(texture);
        } catch (const std::exception &e)
        {
            Console::LogError("[ Material ] %s", e.what());
        }
        
        auto psize = path.size();
        if (!stream->write(reinterpret_cast<const char *>(&psize), sizeof(std::size_t)))
        {
            return;
        }

        if (!stream->write(path.c_str(), psize * sizeof(char)))
        {
            return;
        }
    }

    // write shader uniform
    auto usize = m_field_list.size();
    if (!stream->write(&usize, sizeof(std::size_t)))
    {
        return;
    }

    auto uniforms = m_shader->GetUniforms();
    for (auto &&[data, id] : m_field_list)
    {
        auto &uniform = uniforms[id];

        std::string name = uniform.name;
        
        auto nsize = name.size();
        if (!stream->write(&nsize, sizeof(std::size_t)))
        {
            return;
        }

        if (!stream->write(name.c_str(), nsize * sizeof(char)))
        {
            return;
        }
        

        if (!stream->write(&uniform.stride, sizeof(uint32_t)))
        {
            return;
        }

        if (!stream->write(data, uniform.stride))
        {
            return;
        }
    }

    if (!stream->write(m_scalling))
        return;

    if (!stream->write(m_offset))
        return;
}

void Material::deserialize(Stream *stream)
{
    {
        std::string shadername;
        if (!stream->read(shadername))
            return;

        m_shader = AssetsManager::FindShader(shadername);
        if (m_shader == nullptr)
        {
            auto info = AssetsManager::LoadShaderAsync(GetShaderParamsFromName(shadername));
            m_shader = info->get();
        }
    }

    bool isInstancing = false;
    if (!stream->read(isInstancing))
        return;

    if (isInstancing)
    {
        std::string shadername;
        if (!stream->read(shadername))
            return;

        m_shaderInstancing = AssetsManager::FindShader(shadername);
        if (m_shaderInstancing == nullptr)
        {
            auto info = AssetsManager::LoadShaderAsync(GetShaderParamsFromName(shadername));
            m_shaderInstancing = info->get();
        }
    }


    std::size_t tCount = 0;
    if (!stream->read(reinterpret_cast<char *>(&tCount), sizeof(std::size_t)))
    {
        return;
    }

    for (std::size_t i={};i<tCount;i++) {

        int id = 0;
        if (!stream->read(reinterpret_cast<char *>(&id), sizeof(int)))
        {
            return;
        }

        std::size_t psize = 0;
        if (!stream->read(reinterpret_cast<char *>(&psize), sizeof(std::size_t)))
        {
            return;
        }

        std::string path(psize, '\0');
        if (!stream->read(path.data(), psize * sizeof(char)))
        {
            return;
        }

        AssetsManager::LoadTexture(path, &Material::setTexture, this, id);
    }

    // read shader uniform
    std::size_t usize = 0;
    if (!stream->read(reinterpret_cast<char *>(&usize), sizeof(std::size_t)))
    {
        return;
    }
    
    for (std::size_t i={};i<usize;i++)
    {
        std::size_t nsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
        {
            return;
        }

        std::string name(nsize, '\0');
        if (!stream->read(name.data(), nsize * sizeof(char)))
        {
            return;
        }

        uint32_t stride = 0;
        if (!stream->read(reinterpret_cast<char *>(&stride), sizeof(uint32_t)))
        {
            return;
        }

        uint8_t data[stride];
        if (!stream->read(reinterpret_cast<char *>(data), stride))
        {
            return;
        }

        internal_set_value(name.c_str(), 1, data);
    }

    if (!stream->read(m_scalling))
        return;

    if (!stream->read(m_offset))
        return;
}

void Material::flush(grr::gShader* shader) const
{
    for (auto &&[data, id] : m_field_list)
        shader->SetUniform(id, data);

    auto u_transform = m_scalling + m_offset;

    shader->setUniform("u_transform", &u_transform);
}

bool Material::setupTexture(int id, grr::gTexture *texture)
{
    m_textures[id > 10 ? 0 : id] = texture;
    return true;
}

void *Material::internal_set_value(const char *name, u8 count, const void *data)
{
    auto it = m_field_data.find(name);
    if (it == m_field_data.end())
    {
        auto id = m_shader->findUniform(name);
        if (id == GR_INVALID_ID)
            return nullptr;

        auto &uniform = m_shader->GetUniforms()[id];

        auto ptr = std::malloc(uniform.stride);
        
        m_field_data.emplace(name, std::make_tuple(ptr, uniform.stride));

        m_field_list.emplace_back(ptr, id);
    }

    auto &&[ptr, stride] = m_field_data[name];

    if (data != nullptr)
    {
        memcpy(ptr, data, stride);
    }

    return ptr;
}






