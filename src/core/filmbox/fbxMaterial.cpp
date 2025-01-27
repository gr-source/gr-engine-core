#include "gr-base/core/filmbox/fbxMaterial.hpp"

FbxMaterial::FbxMaterial(const std::string & name) : m_name(name) {}

const Color &FbxMaterial::getColor() const {
    return albedo;
}

const std::string &FbxMaterial::getName() const {
    return m_name;
}

const std::unordered_map<TextureType, const std::string> &FbxMaterial::getTextures() const {
    return m_textures;
}
