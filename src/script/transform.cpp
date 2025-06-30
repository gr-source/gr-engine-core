#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/file/stream.hpp"

#include <algorithm>

Transform::Transform() : m_scale(Vector3::one), m_rotation(Quaternion::identity), m_position(Vector3::zero), m_parent(nullptr), m_localMatrix(Matrix4x4::identityMatrix), m_globalMatrix(Matrix4x4::identityMatrix)
{}

void Transform::Start()
{
    m_lastposition = m_position;
    m_lastrotation = m_rotation;
    m_lastscale = m_scale;
}

void Transform::Destroy()
{
    m_position = m_lastposition;
    m_rotation = m_lastrotation;
    m_scale = m_lastscale;

    updateLocalMatrix();
}

void Transform::setLocalScale(const Vector3 &value) {
    m_scale = value;

    updateLocalMatrix();
}

Vector3 Transform::getLocalScale() const {
    return m_scale;
}

void Transform::setLocalRotation(const Quaternion &value) {
    m_rotation = value;

    updateLocalMatrix();
}

Quaternion Transform::getLocalRotation() const {
    return m_rotation;
}

Quaternion Transform::getRotation() const {
    if (m_parent) {
        return m_parent->m_rotation * m_rotation;
    }
    return m_rotation;
}

void Transform::setLocalPosition(const Vector3 &value) {
    m_position = value;

    updateLocalMatrix();
}

Vector3 Transform::getLocalPosition() const {
    return m_position;
}

void Transform::addLocalPosition(const Vector3 &value) {
    m_position += value;

    updateLocalMatrix();
}

Vector3 Transform::getPosition() const {
    if (m_parent) {
        return Vector3(m_parent->getGlobalMatrix() * Vector4({m_position.x, m_position.y, m_position.z, 1.0f}));
    }
    return m_position;
}

Vector3 Transform::getLocalForward() const {
    return m_rotation * Vector3::forward;
}

Vector3 Transform::getForward() const {
    return getRotation() * Vector3::forward;
}

Vector3 Transform::getRight() const {
    return getRotation() * Vector3::right;
}

Vector3 Transform::getUp() const {
    return getRotation() * Vector3::up;
}

Matrix4x4 Transform::getGlobalMatrix() const {
    return m_globalMatrix;
}

Matrix4x4 Transform::getLocalMatrix() const {
    return m_localMatrix;
}

bool Transform::hasParent() const {
    return m_parent != nullptr;
}

void Transform::setParent(Transform *transform, bool remove) {
    if (remove && m_parent != nullptr) {
        m_parent->m_children.erase(std::remove(m_parent->m_children.begin(), m_parent->m_children.end(), this), m_parent->m_children.end());
    }
    
    if (transform != nullptr)
        transform->addChildren(this);
    m_parent = transform;

    updateLocalMatrix();
}

Transform *Transform::getParent() const {
    return m_parent;
}

void Transform::addChildren(Transform *child) {
    if (!child || std::find(m_children.begin(), m_children.end(), child) != m_children.end()) {
        return;
    }
    
    m_children.push_back(child);
}

std::vector<Transform *> Transform::getChildren() const {
    return m_children;
}

bool Transform::hasChildren() const {
    return !m_children.empty();
}

void Transform::updateLocalMatrix()
{
    m_localMatrix = Math::CreateTRS(m_position, m_rotation, m_scale);

    updateGlobalMatrix();
}

void Transform::updateGlobalMatrix()
{
    m_globalMatrix = getLocalMatrix();

    if (m_parent)
    {
        m_globalMatrix = m_parent->getGlobalMatrix() * m_globalMatrix;
    }

    for (auto & child : m_children)
    {
        child->updateLocalMatrix();
    }
}

Transform *Transform::findChild(Transform *transform) const {
    Transform *result = nullptr;
    for (auto &child : m_children) {
        if (child == transform) {
            result = child;
            break;
        }
        
        result = child->findChild(transform);
        if (result) {
            break;
        }
    }
    return result;
}

Transform *Transform::findChildrenComponent(const std::string &name) const {
    Transform *result = nullptr;
    for (auto &child : m_children) {
        if (child->entity->hasComponent(name)) {
            result = child;
            break;
        }
        
        result = child->findChildrenComponent(name);
        if (result) {
            break;
        }
    }
    return result;
}

bool Transform::Serialize(Stream *stream) {
    if (!stream->write(reinterpret_cast<const char *>(&m_scale), sizeof(Vector3)))
    {
        return false;
    }

    if (!stream->write(reinterpret_cast<const char *>(&m_rotation), sizeof(Quaternion)))
    {
        return false;
    }

    if (!stream->write(reinterpret_cast<const char *>(&m_position), sizeof(Vector3)))
    {
        return false;
    }
    return true;
}

bool Transform::Deserialize(Stream *stream)
{
    if (!stream->read(reinterpret_cast<char *>(&m_scale), sizeof(Vector3)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_rotation), sizeof(Quaternion)))
    {
        return false;
    }

    if (!stream->read(reinterpret_cast<char *>(&m_position), sizeof(Vector3)))
    {
        return false;
    }

    updateLocalMatrix();
    return true;
}



