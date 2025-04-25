#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/geometry/shader.hpp"

#include <gr-render/gShader.h>

Material::Material() : m_shader(nullptr)
{
    for (int i=0;i<10;i++)
    {
        m_textures[i] = nullptr;
    }
}

Material::~Material()
{
    for (auto &&[name, ptr] : m_uniformMap)
    {
        if (ptr != nullptr)
        {
            std::free(ptr);
        }
    }
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

grr::gShader *Material::getShader() const
{
    return m_shader;
}

void Material::setShader(grr::gShader *shader)
{

    m_shader = shader;

    std::vector<std::tuple<std::string, std::unique_ptr<uint8_t>>> temp;

    for (auto &&[name, ptr] : m_uniformMap)
    {
        temp.emplace_back(name, std::unique_ptr<uint8_t>((uint8_t *)ptr));
    }

    m_uniformMap.clear();
    m_uniformList.clear();

    for (auto &&[name, data] : temp)
    {
        if (internal_set_value(name.c_str(), 0, data.get()) != nullptr)
        {
            continue;
        }

        Console::LogError("[ Material ] Fail to set value: %s", name.c_str());
    }
}

void Material::serialize(Stream *stream) const
{
    bool hasShader = m_shader != nullptr;

    if (!stream->write(&hasShader, sizeof(bool)))
    {
        return;
    }

    if (hasShader)
    {
        auto name = AssetsManager::GetShaderName(m_shader);

        auto nsize = name.size();
        if (!stream->write(&nsize, sizeof(std::size_t)))
        {
            return;
        }

        if (!stream->write(name.c_str(), nsize * sizeof(char)))
        {
            return;
        }
    }

    std::vector<std::tuple<grr::gTexture *, int>> textures;
    for (std::size_t i={};i<m_textures.size();i++)
    {
        if (m_textures[i] == nullptr)
        {
            continue;
        }
        textures.emplace_back(m_textures[i], i);
    }

    auto tCount = textures.size();
    if (!stream->write(&tCount, sizeof(std::size_t)))
    {
        return;
    }

    for (auto &&[texture, id] : textures) {
        if (!stream->write(reinterpret_cast<const char *>(&id), sizeof(int)))
        {
            return;
        }

        std::string path;
        try {
            path = AssetsManager::GetRefTextureName(texture);
        } catch (const std::exception &e) {
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
    auto usize = m_uniformList.size();
    if (!stream->write(&usize, sizeof(std::size_t)))
    {
        return;
    }
    
    for (auto &&[id, data] : m_uniformList)
    {
        auto &uniform = m_shader->getUniform()[id];

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
}

void Material::deserialize(Stream *stream)
{
    bool hasShader = false;
    if (!stream->read(reinterpret_cast<char *>(&hasShader), sizeof(bool)))
    {
        return;
    }

    std::string shader_path;
    if (hasShader)
    {
        std::size_t nsize = 0;
        if (!stream->read(reinterpret_cast<char *>(&nsize), sizeof(std::size_t)))
        {
            return;
        }

        std::string shader_name(nsize, '\0');
        if (!stream->read(shader_name.data(), nsize * sizeof(char)))
        {
            return;
        }

        shader_path = shader_name;

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

        auto it = m_uniformMap.find(name);
        if (it != m_uniformMap.end())
        {
            std::memcpy(it->second, data, stride);
        } else
        {
            auto ptr = std::malloc(stride);
            
            std::memcpy(ptr, data, stride);
            
            m_uniformMap.emplace(name, ptr);
        }
    }

    // if (hasShader)
    // {
        // AssetsManager::LoadShader(ShaderOptimize_DYNAMIC, shader_path, &Material::setShader, this);
    // }
}

void Material::flush() const
{
    for (auto &&[id, data] : m_uniformList)
    {
        m_shader->setUniform(id, data);
    }
}

bool Material::setupTexture(int id, grr::gTexture *texture)
{
    m_textures[id > 10 ? 0 : id] = texture;
    return true;
}

void *Material::internal_set_value(const char *name, grm::u8 count, const void *data)
{
    auto id = m_shader->findUniform(name);
    if (id == -1)
    {
        return nullptr;
    }

    auto &uniform = m_shader->getUniform()[id];

    void *ptr = nullptr;
    auto it = m_uniformMap.find(name);
    if (it == m_uniformMap.end())
    {
        ptr = std::malloc(uniform.stride);

        m_uniformList.emplace_back(id, ptr);

        m_uniformMap.emplace(name, ptr);

    } else
    {
        ptr = it->second;

        if (!ptr)
        {
            m_uniformMap.erase(it);
            return internal_set_value(name, count, data);
        }
    }
 
    std::memcpy(ptr, data, uniform.stride);

    return ptr;
}






