#pragma once

#include "gr-base/file/default.hpp"

#include <gr-math/gmath.hpp>

class Bone {
public:
    Bone();

    Vector3 getPosition() const;

    void setPosition(const Vector3 &position);

    Quaternion getRotation() const;

    void setRotation(const Quaternion& rotation);

    Vector3 getScale() const;

    void setScale(const Vector3 &scale);

    Matrix4x4 getLocalMatrix() const;

    int getParentIndex() const;

    void setParentIndex(int index);

    bool serialize(Stream *stream) const;

    bool deserialize(Stream *stream);
    
private:
    Vector3 m_position;

    Quaternion m_rotation;

    Vector3 m_scale;

    Matrix4x4 m_localMatrix;

    int m_parentID;

    void updateMatrix();
};

