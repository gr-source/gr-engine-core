#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"

Material::Material() : m_shader(nullptr)
{
    // AssetsManager::LoadShader(File::GetAsset("/shader/model/default_instancing_model.glsl"), &Material::setShader, this);

    for (int i=0;i<10;i++)
    {
        m_texture[i] = nullptr;
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

void Material::setTexture(int idx, grr::gTexture *texture) {
    m_texture[idx >= m_texture.size() ? 0 : idx] = texture;
}

grr::gTexture *Material::getTexture(int idx) const {
    return m_texture[idx >= m_texture.size() ? 0 : idx];
}

std::array<grr::gTexture *, 10> Material::getTextures() const
{
    return m_texture;
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
        internal_set_value(name.c_str(), 0, data.get());
    }
}

void Material::serialize(Stream *stream) const
{
    std::vector<std::tuple<grr::gTexture *, int>> textures;
    for (std::size_t i={};i<m_texture.size();i++)
    {
        if (m_texture[i] == nullptr)
        {
            continue;
        }
        textures.emplace_back(m_texture[i], i);
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

        uint8_t data[stride] = {0};
        if (!stream->read(reinterpret_cast<char *>(data), stride))
        {
            return;
        }

        if (m_uniformMap[name] != nullptr)
        {
            std::memcpy(m_uniformMap[name], data, stride);
        } else
        {
            m_uniformMap[name] = std::malloc(stride);
            
            std::memcpy(m_uniformMap[name], data, stride);
        }
    }
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
    m_texture[id > 10 ? 0 : id] = texture;
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
    
    if (m_uniformMap[name] == nullptr)
    {
        m_uniformMap[name] = std::malloc(uniform.stride);

        m_uniformList.emplace_back(id, m_uniformMap[name]);
    }

    std::memcpy(m_uniformMap[name], data, uniform.stride);

    return m_uniformMap[name];
}
