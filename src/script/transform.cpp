#include "gr-base/script/transform.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/gameObject.hpp"
#include "gr-base/ecs/behaviorStorage.hpp"

#include <algorithm>

Transform::Transform() : m_scale(Vector3::one), m_rotation(Quaternion::identity), m_position(Vector3::zero), m_parent(HANDLE_INVALID), m_localMatrix(Matrix4x4::identityMatrix), m_globalMatrix(Matrix4x4::identityMatrix), isDirty(false)
{}

void Transform::setLocalScale(const Vector3 &value)
{
    m_scale = value;

    updateLocalMatrix();
}

const Vector3 &Transform::getLocalScale() const
{
    return m_scale;
}

void Transform::setLocalRotation(const Quaternion &value)
{
    m_rotation = value;

    updateLocalMatrix();
}

const Quaternion &Transform::getLocalRotation() const
{
    return m_rotation;
}

Quaternion Transform::getRotation() const
{
    if (m_parent != HANDLE_INVALID)
    {
        auto parent = BehaviorStorage::GetComponent<Transform>(m_parent);

        return parent->getRotation() * m_rotation;
    }
    return m_rotation;
}

void Transform::setLocalPosition(const Vector3 &value)
{
    m_position = value;

    updateLocalMatrix();
}

const Vector3 &Transform::getLocalPosition() const
{
    return m_position;
}

Vector3 Transform::getPosition() const
{
    return {
        m_globalMatrix[3][0],
        m_globalMatrix[3][1],
        m_globalMatrix[3][2]
    };
}

Vector3 Transform::getLocalForward() const
{
    return m_rotation * Vector3::forward;
}

Vector3 Transform::getForward() const
{
    return getRotation() * Vector3::forward;
}

Vector3 Transform::getRight() const {
    return getRotation() * Vector3::right;
}

Vector3 Transform::getUp() const {
    return getRotation() * Vector3::up;
}

const Matrix4x4 &Transform::getGlobalMatrix() const
{
    return m_globalMatrix;
}

const Matrix4x4 &Transform::getLocalMatrix() const {
    return m_localMatrix;
}

bool Transform::hasParent() const {
    return m_parent != HANDLE_INVALID;
}

void Transform::setParent(Transform* newParent, bool)
{
    ComponentHandle handle = newParent != nullptr ? newParent->GetGameObject()->GetID<Transform>() : HANDLE_INVALID;
    if (m_parent == handle)
        return;

    if (m_parent != HANDLE_INVALID)
    {
        auto parent = BehaviorStorage::GetComponent<Transform>(m_parent);

        auto& children = parent->m_children;
        children.erase(std::remove(children.begin(), children.end(), GetGameObject()->GetID<Transform>()), children.end());
    }

    m_parent = handle;

    if (m_parent != HANDLE_INVALID)
    {
        auto parent = BehaviorStorage::GetComponent<Transform>(m_parent);

        auto& children = parent->m_children;
        children.push_back(GetGameObject()->GetID<Transform>());
    }

    updateLocalMatrix(); 
}

Transform *Transform::getParent() const
{
    return m_parent != HANDLE_INVALID ? BehaviorStorage::GetComponent<Transform>(m_parent) : nullptr;
}

void Transform::addChildren(Transform *child)
{
    child->m_parent = child->GetGameObject()->GetID<Transform>();

    m_children.push_back(child->GetGameObject()->GetID<Transform>());
}

std::vector<ComponentHandle> Transform::getChildren() const
{
    return m_children;
}

bool Transform::hasChildren() const
{
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

    if (m_parent != HANDLE_INVALID)
    {
        auto parent = BehaviorStorage::GetComponent<Transform>(m_parent);

        m_globalMatrix = parent->getGlobalMatrix() * m_globalMatrix;
    }

    for (auto &handle : m_children)
    {
        auto transform = BehaviorStorage::GetComponent<Transform>(handle);
        transform->updateGlobalMatrix();
    }

    isDirty = true;
}

bool Transform::Serialize(Stream *stream) const
{
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



