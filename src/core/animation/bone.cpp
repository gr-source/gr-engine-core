#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"

Bone::Bone() :
    m_position(type_traits<Vector3>::zero),
    m_rotation(type_traits<Quaternion>::identity),
    m_scale(type_traits<Vector3>::one),
    m_localMatrix(type_traits<Matrix4x4>::identity),
    m_parentID(0)
{
    updateMatrix();
}

Vector3 Bone::getPosition() const
{
    return m_position;
}

void Bone::setPosition(const Vector3 &position)
{
    m_position = position;

    updateMatrix();
}

Quaternion Bone::getRotation() const
{
    return m_rotation;
}

void Bone::setRotation(const Quaternion &rotation)
{
    m_rotation = rotation;

    updateMatrix();
}

Vector3 Bone::getScale() const
{
    return m_scale;
}

void Bone::setScale(const Vector3 &scale)
{
    m_scale = scale;

    updateMatrix();
}

Matrix4x4 Bone::getLocalMatrix() const {
    return m_localMatrix;
}

int Bone::getParentIndex() const {
    return m_parentID;
}

void Bone::setParentIndex(int index)
{
    m_parentID = index;
}

void Bone::updateMatrix() {
    m_localMatrix = Math::CreateTRS(m_position, m_rotation, m_scale);
}

bool Bone::serialize(Stream *stream) const {
    // write transform
    {
        if (!stream->write(&m_position, sizeof(Vector3))) {
            return false;
        }

        if (!stream->write(&m_rotation, sizeof(Quaternion))) {
            return false;
        }

        if (!stream->write(&m_scale, sizeof(Vector3))) {
            return false;
        }
    }

    // bone index
    if (!stream->write(&m_parentID, sizeof(int))) {
        return false;
    }

    return true;
}

bool Bone::deserialize(Stream *stream) {
    // read transform
    {
        if (!stream->read(reinterpret_cast<char *>(&m_position), sizeof(Vector3))) {
            return false;
        }

        if (!stream->read(reinterpret_cast<char *>(&m_rotation), sizeof(Quaternion))) {
            return false;
        }

        if (!stream->read(reinterpret_cast<char *>(&m_scale), sizeof(Vector3))) {
            return false;
        }
    }

    // bone index
    if (!stream->read(reinterpret_cast<char *>(&m_parentID), sizeof(int))) {
        return false;
    }

    updateMatrix();

    return true;
}

