#include "core/filmbox/fbxNode.hpp"

FbxNode::FbxNode(const std::string &name, FbxNode *parent) : m_name(name), m_parent(parent) {}

std::string FbxNode::getName() const {
    return m_name;
}

std::vector<FbxMesh> FbxNode::getMeshes() const {
    return m_meshes;
}

bool FbxNode::hasMeshe() const {
    return !m_meshes.empty();
}

const std::vector<FbxNode> &FbxNode::getChildren() const {
    return m_children;
}

void FbxNode::setPosition(const Vector3 &position) {
    m_position = position;
}

const Vector3 &FbxNode::getPosition() const {
    return m_position;
}

void FbxNode::setScale(const Vector3 &scale) {
    m_scale = scale;
}

const Vector3 &FbxNode::getScale() const {
    return m_scale;
}

void FbxNode::setRotation(const Quaternion &rotation) {
    m_rotation = rotation;
}

const Quaternion &FbxNode::getRotation() const {
    return m_rotation;
}
