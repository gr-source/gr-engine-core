#pragma once

#include "gr-base/physics/default.hpp"

#include "gr-base/ecs/entity.hpp"

#include "default.hpp"

class Rigidbody : public Component {
    CLASS_FUNCTIONS
public:
    Rigidbody();

    void Init() override;

    void Start() override;

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

    void applyImpulse(const Vector3 &impulse, const Vector3 &pos);

    void applyCentralImpulse(const Vector3 &impulse);

    void applyCentralForce(const Vector3 &force);

    void applyCentralPushImpulse(const Vector3 &impulse);

    void applyPushImpulse(const Vector3 &impulse, const Vector3 &pos);

    void applyTorqueImpulse(const Vector3 &torque);

    void applyTorqueTurnImpulse(const Vector3 &torque);

    void applyDamping(float timeStep);

    void applyForce(const Vector3 &force, const Vector3 &pos);

    void applyTorque(const Vector3 &torque);

    void setAngularVelocity(const Vector3 &value);

    Vector3 getAngularVelocity() const;

    void setLinearVelocity(const Vector3 &value);

    Vector3 getLinearVelocity() const;

    void setPosition(const Vector3 &position);

    Vector3 getPosition() const;

    void setRotation(const Quaternion &rotation);

    Quaternion getRotation() const;

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



