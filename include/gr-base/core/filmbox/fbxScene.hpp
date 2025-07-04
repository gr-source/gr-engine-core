#pragma once

#include "fbxAnimation.hpp"
#include "fbxMaterial.hpp"
#include "fbxMesh.hpp"
#include "fbxNode.hpp"

#include <algorithm>

class FbxScene {
public:
    FbxScene(const std::string &name = "default");

    template <typename ...Args>
    FbxAnimation &addAnimation(Args &&...args);

    const std::vector<FbxAnimation> &getAnimations() const;

    template <typename ...Args>
    FbxMaterial &addMaterial(Args &&...args);

    const FbxMaterial &getMaterial(int idx) const;

    const std::vector<FbxMaterial> &getMaterials() const;

    const std::size_t getMaterialCount() const;

    template <typename ...Args>
    FbxMesh *addMesh(Args &&...args);

    FbxMesh *getMesh(int index) const;

    const std::size_t getMeshCount() const;

    const FbxNode& getRootNode() const;

    const FbxNode *findNode(const std::string &name) const;

    const std::string &getName() const;
    
private:
    std::string m_name;

    std::vector<FbxAnimation> m_animation;

    std::vector<FbxMaterial> m_materials;

    std::vector<std::unique_ptr<FbxMesh>> m_meshes;

    FbxNode m_rootNode;

    const FbxNode *readNode(const FbxNode& node, const std::string &name) const;

    friend class Filmbox;
};

template <typename... Args>
inline FbxAnimation &FbxScene::addAnimation(Args &&...args) {
    return m_animation.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxMaterial &FbxScene::addMaterial(Args &&...args) {
    return m_materials.emplace_back(std::forward<Args>(args)...);
}

template <typename... Args>
inline FbxMesh *FbxScene::addMesh(Args &&...args) {
    return &(*m_meshes.emplace_back(std::make_unique<FbxMesh>(std::forward<Args>(args)...)));
}


