#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"
#include <gr-math/quaternion.hpp>
#include <gr-math/vector3.hpp>
#include <string>

Bone::Bone(const std::string &name, const Vector3 &position, const Vector3 &scale, const Quaternion &rotation, int id) : m_name(name), m_position(position), m_scale(scale), m_rotation(rotation), m_parent(id), m_localMatrix(Matrix4x4::identityMatrix)
{
    updateMatrix();
}

std::string Bone::getName() const {
    return m_name;
}

void Bone::setName(const std::string &name)
{
    m_name = name;
}

Matrix4x4 Bone::getLocalMatrix() const {
    return m_localMatrix;
}

int Bone::getParentIndex() const {
    return m_parent;
}

void Bone::setParentIndex(int index)
{
    m_parent = index;
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

Quaternion Bone::getRotation() const
{
    return m_rotation;
}

void Bone::setRotation(const Quaternion &rotation)
{
    m_rotation = rotation;

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

void Bone::updateMatrix() {
    m_localMatrix = Math::CreateTRS(m_scale, m_rotation, m_position); // Math::translate<Matrix4x4>(m_position) * Math::rotate<Matrix4x4>(Math::normalize(m_rotation)) * Math::scale<Matrix4x4>(m_scale);
}

bool Bone::serialize(Stream *stream) const {
    // Write name bone
    {
        auto nlen = m_name.size();
        if (!stream->write(&nlen, sizeof(std::size_t))) {
            return false;
        }

        if (!stream->write(m_name.c_str(), nlen * sizeof(char))) {
            return false;
        }
    }

    /*
    int m_boneIndex;
    */

    // write transform
    {
        if (!stream->write(&m_scale, sizeof(Vector3))) {
            return false;
        }

        if (!stream->write(&m_rotation, sizeof(Quaternion))) {
            return false;
        }

        if (!stream->write(&m_position, sizeof(Vector3))) {
            return false;
        }
    }

    // bone index
    if (!stream->write(&m_parent, sizeof(int))) {
        return false;
    }

    return true;
}

bool Bone::deserialize(Stream *stream) {
    // read name bone
    {
        std::size_t nlen = 0;
        if (!stream->read(reinterpret_cast<char *>(&nlen), sizeof(std::size_t))) {
            return false;
        }

        m_name.resize(nlen, '\0');
        if (!stream->read(m_name.data(), nlen * sizeof(char))) {
            return false;
        }
    }

    /*
    int m_boneIndex;
    */

    // read transform
    {
        if (!stream->read(reinterpret_cast<char *>(&m_scale), sizeof(Vector3))) {
            return false;
        }

        if (!stream->read(reinterpret_cast<char *>(&m_rotation), sizeof(Quaternion))) {
            return false;
        }

        if (!stream->read(reinterpret_cast<char *>(&m_position), sizeof(Vector3))) {
            return false;
        }
    }

    // bone index
    if (!stream->read(reinterpret_cast<char *>(&m_parent), sizeof(int))) {
        return false;
    }

    updateMatrix();

    return true;
}

