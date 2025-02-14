#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"

Material::Material(const Color &albedo, float metallic, float roughness, float ao) : m_albedo(albedo), m_metallic(metallic), m_roughness(roughness), m_ao(ao), m_shader(nullptr) {
    AssetsManager::LoadShader(File::GetAsset("/shader/model/default_instancing_model.glsl"), &Material::setShader, this);

    for (int i=0;i<10;i++)
    {
        m_texture[i] = nullptr;
    }
}

Color Material::getAlbedo() const {
    return m_albedo;
}

void Material::setAlbedo(const Color &color) {
    m_albedo = color;
}

float Material::getMetallic() const {
    return m_metallic;
}

void Material::setMetallic(float value) {
    m_metallic = value;
}

float Material::getRoughness() const {
    return m_roughness;
}

void Material::setRoughness(float value) {
    m_roughness = value;
}

float Material::getAmbientOcclusion() const {
    return m_ao;
}

void Material::setAmbientOcclusion(float value) {
    m_ao = value;
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
}

void Material::serialize(Stream *stream) const
{
    // serialize albedo
    stream->write(reinterpret_cast<const char *>(&m_albedo), sizeof(Vector4));

    // serialize metallic
    stream->write(reinterpret_cast<const char *>(&m_metallic), sizeof(float));

    // serialize roughness
    stream->write(reinterpret_cast<const char *>(&m_roughness), sizeof(float));

    // serialize ao
    stream->write(reinterpret_cast<const char *>(&m_ao), sizeof(float));

    auto tSize = m_texture.size();
    stream->write(reinterpret_cast<const char *>(&tSize), sizeof(std::size_t));

    for (int i = 0;i<m_texture.size();i++) {
        stream->write(reinterpret_cast<const char *>(&i), sizeof(int));
        std::string name;
        if (m_texture[i] != nullptr) {
            try {
                name = AssetsManager::GetRefTextureName(m_texture[i]);
            } catch (const std::exception &e) {
                Console::LogError("[ Material ] %s", e.what());
            }
        }
        
        std::size_t psize = name.size();
        stream->write(reinterpret_cast<const char *>(&psize), sizeof(std::size_t));
        stream->write(name.data(), psize * sizeof(char));
    }
}

void Material::deserialize(Stream *stream) {
    // deserialize albedo
    stream->read(reinterpret_cast<char *>(&m_albedo), sizeof(Vector4));

    // deserialize metallic
    stream->read(reinterpret_cast<char *>(&m_metallic), sizeof(float));

    // deserialize roughness
    stream->read(reinterpret_cast<char *>(&m_roughness), sizeof(float));

    // deserialize ao
    stream->read(reinterpret_cast<char *>(&m_ao), sizeof(float));

    std::size_t tSize = 0;
    stream->read(reinterpret_cast<char *>(&tSize), sizeof(std::size_t));

    for (int i=0;i<tSize;i++) {
        int id = 0;
        stream->read(reinterpret_cast<char *>(&id), sizeof(int));
        
        std::size_t psize = 0;
        stream->read(reinterpret_cast<char *>(&psize), sizeof(std::size_t));

        std::string name(psize, '\0');
        stream->read(name.data(), psize * sizeof(char));

        if (name.empty()) {
            continue;
        }
        AssetsManager::LoadTexture(name, &Material::setupTexture, this, id);
    }
}

bool Material::setupTexture(int id, grr::gTexture *texture) {
    m_texture[id] = texture;
    return true;
}
