#pragma once

#include "gr-base/ecs/behavior.hpp"

#include <gr-math/gmath.hpp>

#include "gr-base/core/file/default.hpp"

class Transform : public BehaviorImpl<Transform>
{
public:
    Transform();

    // void onStart() override;

    // void onDestroy() override;

    void setLocalScale(const Vector3 &value);

    const Vector3 &getLocalScale() const;

    void setLocalRotation(const Quaternion &value);

    const Quaternion &getLocalRotation() const;

    Quaternion getRotation() const;

    void setLocalPosition(const Vector3 &value);

    void addLocalPosition(const Vector3 &value);

    Vector3 getPosition() const;

    const Vector3 &getLocalPosition() const;

    Vector3 getRight() const;

    Vector3 getUp() const;

    Vector3 getLocalForward() const;

    Vector3 getForward() const;

    void setGlobalMatrix(const Matrix4x4 &matrix);

    const Matrix4x4 &getGlobalMatrix() const;

    const Matrix4x4 &getLocalMatrix() const;

    bool hasParent() const;

    void setParent(Transform* transform, bool remove = true);

    Transform *getParent() const;

    void addChildren(Transform *transform);

    std::vector<Transform *> getChildren() const;

    bool hasChildren() const;

    bool Serialize(Stream *stream) const override;
    
    bool Deserialize(Stream *stream) override;

private:
    Vector3 m_scale;

    Quaternion m_rotation;

    Vector3 m_position;

    Vector3 m_lastscale;

    Quaternion m_lastrotation;

    Vector3 m_lastposition;

    std::vector<Transform *> m_children;

    Transform* m_parent;

    Matrix4x4 m_localMatrix;

    Matrix4x4 m_globalMatrix;

    void updateLocalMatrix();

    void updateGlobalMatrix();
};

REGISTER_COMPONENT(Transform)



