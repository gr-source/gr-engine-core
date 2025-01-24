#include "core/geometry/material.hpp"

#include "core/file/stream.hpp"

#include "core/assetsManager.hpp"

Material::Material(const std::string &name, const Color &albedo, float metallic, float roughness, float ao) : m_name(name), m_albedo(albedo), m_metallic(metallic), m_roughness(roughness), m_ao(ao), m_texture(10, nullptr) {}

std::string Material::getName() const {
    return m_name;
}

void Material::setName(const std::string &name) {
    m_name = name;
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

void Material::serialize(Stream *stream) const {
    // serialize name
    auto nSize = m_name.size();
    stream->write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));
    stream->write(m_name.data(), nSize * sizeof(char));

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
        auto name = AssetsManager::GetRefTextureName(m_texture[i]);

        nSize = name.size();
        stream->write(reinterpret_cast<const char *>(&nSize), sizeof(std::size_t));
        stream->write(name.data(), nSize * sizeof(char));
    }
}

void Material::deserialize(Stream *stream) {
    // deserialize name
    std::size_t nSize;
    stream->read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));

    m_name.resize(nSize, '\0');
    stream->read(m_name.data(), nSize * sizeof(char));

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
        stream->read(reinterpret_cast<char *>(&nSize), sizeof(std::size_t));

        std::string name(nSize, '\0');
        stream->read(name.data(), nSize * sizeof(char));

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
