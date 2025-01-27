#pragma once

#include "gr-base/physics/default.hpp"

#include "gr-base/ecs/entity.hpp"

#include "default.hpp"

class Rigidbody : public Component {
    CLASS_FUNCTIONS
public:
    Rigidbody();

    void Init() override;

    void Destroy() override;

    void Update() override;

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;

    float getMass() const;

    void setMass(float value);

    Vector3 getFreezerRotation() const;

    void setFreezerRotation(const Vector3 &value);

    float getMotionThreshold() const;

    void setMotionThreshold(float value);

    float getSweptSphereRadius() const;

    void setSweptSphereRadius(float value);

private:
    std::shared_ptr<btDefaultMotionState> m_DefaultMotionState;

    std::shared_ptr<btRigidBody> m_body;

    btCollisionShape *m_collisionShape;

    Transform *m_transform;

    float m_mass;

    Vector3 m_lookRotation;

    float fMotionThreshold;

    float fSweptSphereRadius;
};

REGISTER_COMPONENT(Rigidbody)



