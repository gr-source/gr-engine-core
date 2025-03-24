#pragma once

#include <string>

#include "gr-base/core/file/default.hpp"

#include <gr-math/gmath.hpp>

// TODO: Remover o nome do bone dessa classe, inutil?
class Bone {
public:
    Bone(const std::string &name = "default", const Vector3 &position = Vector3::zero, const Vector3 &scale = Vector3::one, const Quaternion &rotation = Quaternion::identity, int id = 0);

    std::string getName() const;

    Matrix4x4 getLocalMatrix() const;

    int getBoneIndex() const;

    Vector3 getScale() const;

    Quaternion getRotation() const;

    Vector3 getPosition() const;

    bool serialize(Stream *stream) const;

    bool deserialize(Stream *stream);
    
private:
    std::string m_name;

    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;

    int m_boneIndex;

    Matrix4x4 m_localMatrix;

    void updateMatrix();
};

