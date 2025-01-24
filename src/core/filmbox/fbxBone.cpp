#include "core/filmbox/fbxBone.hpp"

FbxBone::FbxBone(const std::string & name, int id) : m_name(name), m_id(id) {}

std::string FbxBone::getName() const {
    return m_name;
}

void FbxBone::setPosition(const Vector3 &position) {
    m_position = position;
}

Vector3 FbxBone::getPosition() const {
    return m_position;
}

void FbxBone::setScale(const Vector3 &scale) {
    m_scale = scale;
}

Vector3 FbxBone::getScale() const {
    return m_scale;
}

void FbxBone::setRotation(const Quaternion &rotation) {
    m_rotation = rotation;
}

Quaternion FbxBone::getRotation() const {
    return m_rotation;
}

int FbxBone::getID() const {
    return m_id;
}
