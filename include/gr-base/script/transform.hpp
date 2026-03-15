#pragma once

#include "gr-base/ecs/behavior.hpp"

#include <gr-math/gmath.hpp>

#include "gr-base/core/file/default.hpp"

class Transform : public BehaviorImpl<Transform>
{
public:
    Transform();

    void setLocalScale(const Vector3 &value);

    const Vector3 &getLocalScale() const;

    void setLocalRotation(const Quaternion &value);

    const Quaternion &getLocalRotation() const;

    Quaternion getRotation() const;

    void setLocalPosition(const Vector3 &value);

    Vector3 getPosition();

    const Vector3 &getLocalPosition() const;

    Vector3 getRight() const;

    Vector3 getUp() const;

    Vector3 getLocalForward() const;

    Vector3 getForward() const;

    void setGlobalMatrix(const Matrix4x4 &matrix);

    Matrix4x4 &getGlobalMatrix();

    const Matrix4x4 &getLocalMatrix() const;

    bool hasParent() const;

    void setParent(Transform* transform, bool remove = true);

    Transform *getParent() const;

    void addChildren(Transform *transform);

    std::vector<ComponentHandle> getChildren() const;

    bool hasChildren() const;

    bool Serialize(Stream *stream) const override;
    
    bool Deserialize(Stream *stream) override;

private:
    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;

    std::vector<ComponentHandle> m_children;

    ComponentHandle m_parent;

    Matrix4x4 m_localMatrix;

    Matrix4x4 m_globalMatrix;

    bool isDirty;

    void updateLocalMatrix();

    void updateGlobalMatrix();
};

REGISTER_COMPONENT(Transform, (uint64_t)-1)



