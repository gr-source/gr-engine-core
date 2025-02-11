#pragma once

#include <string>
#include <vector>

#include "fbxMesh.hpp"

class FbxNode {
public:
    FbxNode(const std::string &name = "default", FbxNode *parent = nullptr);

    std::string getName() const;

    void addMesh(FbxMesh *mesh);

    std::vector<FbxMesh *> getMeshes() const;

    bool hasMeshe() const;

    template <typename ...Args>
    FbxNode &addNode(Args &&...args);

    const std::vector<FbxNode> &getChildren() const;

    void setPosition(const Vector3 &position);

    const Vector3 &getPosition() const;

    void setScale(const Vector3 &scale);

    const Vector3 &getScale() const;

    void setRotation(const Quaternion &rotation);

    const Quaternion &getRotation() const;

private:
    std::string m_name;

    std::vector<FbxNode> m_children;

    std::vector<FbxMesh *> m_meshes;

    FbxNode *m_parent;

    Vector3 m_position;

    Vector3 m_scale;
    
    Quaternion m_rotation;
};

template <typename... Args>
inline FbxNode &FbxNode::addNode(Args &&...args) {
    return m_children.emplace_back(std::forward<Args>(args)...);
}
