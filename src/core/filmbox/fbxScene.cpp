#include "core/filmbox/fbxScene.hpp"

FbxScene::FbxScene(const std::string & name) : m_name(name) {}

const std::vector<FbxAnimation> &FbxScene::getAnimations() const {
    return m_animation;
}

const FbxMaterial &FbxScene::getMaterial(int idx) const {
    if (idx >= m_materials.size()) {
        throw std::runtime_error("[ FbxMaterial ] Falied to search material. id=" + std::to_string(idx));
    }
    return m_materials[idx];
}

const std::vector<FbxMaterial> &FbxScene::getMaterials() const {
    return m_materials;
}

const std::size_t FbxScene::getMaterialCount() const {
    return m_materials.size();
}

FbxMesh *FbxScene::findMesh(const std::string &name) {
    auto it = m_meshes.find(name);
    if (it == m_meshes.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::size_t FbxScene::getMeshCount() const {
    return m_meshes.size();
}

const FbxNode &FbxScene::getRootNode() const {
    return m_rootNode;
}

const FbxNode *FbxScene::findNode(const std::string &name) const {
    return readNode(m_rootNode, name);
}

const std::string &FbxScene::getName() const {
    return m_name;
}

const FbxNode *FbxScene::readNode(const FbxNode &node, const std::string &name) const {
    const FbxNode *found = nullptr;
    if (node.getName() == name) {
        found = &node;
        return found;
    }
    for (const auto &child : node.getChildren()) {
        found = readNode(child, name);
        if (found) {
            break;
        }
    }
    return found;
}
