#include "gr-base/script/rigidbody.hpp"

#include "LinearMath/btVector3.h"
#include "gr-base/script/meshCollider.hpp"
#include "gr-base/script/boxCollider.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/physics/physicsScene.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/core/gameObject.hpp"

#include "gr-base/core/file/stream.hpp"
#include <cstddef>
#include <gr-math/quaternion.hpp>
#include <gr-math/vector3.hpp>

#define BTVEC3_TO_VEC3(v) Vector3{v.getX(), v.getY(), v.getZ()}

#define BTQUAT_TO_QUAT(q) Quaternion{q.w(), q.x(), q.y(), q.z()}

Rigidbody::Rigidbody() : m_collisionShape(nullptr), m_DefaultMotionState(nullptr), m_body(nullptr), m_transform(nullptr), m_mass(0.0f), m_lookRotation(Vector3::one), fMotionThreshold(0.0f), fSweptSphereRadius(0.0f)
{}

Rigidbody::~Rigidbody()
{
    if (!m_body)
        return;

    PhysicsScene::RemoveRigidbody(&(*m_body));
}

void Rigidbody::Start()
{
    m_transform = GetEntity()->getComponent<Transform>();

    auto meshCollider = GetEntity()->getComponent<MeshCollider>();
    if (meshCollider != NULL)
    {
        m_collisionShape = meshCollider->getShape();
    } else if (GetEntity()->getComponent<BoxCollider>() != nullptr)
    {
        m_collisionShape = GetEntity()->getComponent<BoxCollider>()->getShape();
    }

    if (!m_collisionShape) {
        return Console::LogError("[ Rigidbody ] Shape not found: %s", GetGameObject()->getName().c_str());
    }

    m_DefaultMotionState = std::make_shared<btDefaultMotionState>(
		btTransform(
			QUAT_TO_BTQUAT(m_transform->getRotation()),
			VEC3_TO_BTVEC3(m_transform->getPosition())
		)
	);

    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
    m_collisionShape->calculateLocalInertia(m_mass, fallInertia);

    btRigidBody::btRigidBodyConstructionInfo pRigidBodyConstructionInfo(m_mass, &(*m_DefaultMotionState), m_collisionShape, fallInertia);
    pRigidBodyConstructionInfo.m_friction = 0.6f;
    pRigidBodyConstructionInfo.m_restitution = 0.0f;
    pRigidBodyConstructionInfo.m_rollingFriction = 0.0f;
    pRigidBodyConstructionInfo.m_spinningFriction = 0.0f;
    pRigidBodyConstructionInfo.m_linearDamping = 0.0f;
    pRigidBodyConstructionInfo.m_angularDamping = 0.05f;

    m_body = std::make_shared<btRigidBody>(pRigidBodyConstructionInfo);
	m_body->setAngularFactor(VEC3_TO_BTVEC3(m_lookRotation));
    m_body->setUserPointer(reinterpret_cast<void *>(GetGameObject()));

    m_body->setSleepingThresholds(0.005f, 0.005f);

    if (fMotionThreshold >= 0.0f)
        m_body->setCcdMotionThreshold(fMotionThreshold);

    if (fSweptSphereRadius >= 0.0f)
        m_body->setCcdSweptSphereRadius(fSweptSphereRadius);

    PhysicsScene::AddRigidbody(&(*m_body));
}

void Rigidbody::Update()
{
    m_transform = GetEntity()->getComponent<Transform>();

    if (!GetGameObject()->GetIsStatic())
    {
        if (m_body->getLinearVelocity().length() < 0.01 && m_body->getAngularVelocity().length() < 0.01) {
            m_body->setActivationState(DISABLE_DEACTIVATION);
        } else {
            m_body->forceActivationState(ACTIVE_TAG);
        }
	}

	if (m_body->isActive())
    {
		btTransform trans;
		m_body->getMotionState()->getWorldTransform(trans);

		m_transform->setLocalPosition({trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()});

		m_transform->setLocalRotation({trans.getRotation().getW(), trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ()});
	}
}

bool Rigidbody::Serialize(Stream *stream) const
{
    stream->write(m_mass);
    stream->write(m_lookRotation);
    stream->write(fMotionThreshold);
    stream->write(fSweptSphereRadius);
    return true;
}

bool Rigidbody::Deserialize(Stream *stream)
{
    stream->read(m_mass);
    stream->read(m_lookRotation);
    stream->read(fMotionThreshold);
    stream->read(fSweptSphereRadius);
    return true;
}

float Rigidbody::getMass() const {
    return m_mass;
}

void Rigidbody::setMass(float value) {
    m_mass = value;
}

void Rigidbody::setFreezerRotation(const Vector3 &value) {
    m_lookRotation = value;

    if (m_body)
        m_body->setAngularFactor(VEC3_TO_BTVEC3(value));
}

Vector3 Rigidbody::getFreezerRotation() const {
    return m_lookRotation;
}

float Rigidbody::getMotionThreshold() const {
    return fMotionThreshold;
}

void Rigidbody::setMotionThreshold(float value) {
    fMotionThreshold = value;
}

float Rigidbody::getSweptSphereRadius() const {
    return fSweptSphereRadius;
}

void Rigidbody::setSweptSphereRadius(float value) {
    fSweptSphereRadius = value;
}

void Rigidbody::applyImpulse(const Vector3 &impulse, const Vector3 &pos)
{
    m_body->applyImpulse(VEC3_TO_BTVEC3(impulse), VEC3_TO_BTVEC3(pos));
}

void Rigidbody::applyCentralImpulse(const Vector3 &impulse)
{
    m_body->applyCentralImpulse(VEC3_TO_BTVEC3(impulse));
}

void Rigidbody::applyCentralForce(const Vector3 &force)
{
    m_body->applyCentralForce(VEC3_TO_BTVEC3(force));
}

void Rigidbody::applyCentralPushImpulse(const Vector3 &impulse)
{
    m_body->applyCentralPushImpulse(VEC3_TO_BTVEC3(impulse));
}

void Rigidbody::applyPushImpulse(const Vector3 &impulse, const Vector3 &pos)
{
    m_body->applyPushImpulse(VEC3_TO_BTVEC3(impulse), VEC3_TO_BTVEC3(pos));
}

void Rigidbody::applyTorqueImpulse(const Vector3 &torque)
{
    m_body->applyTorqueImpulse(VEC3_TO_BTVEC3(torque));
}

void Rigidbody::applyTorqueTurnImpulse(const Vector3 &torque)
{
    m_body->applyTorqueTurnImpulse(VEC3_TO_BTVEC3(torque));
}

void Rigidbody::applyDamping(float timeStep)
{
    m_body->applyDamping(timeStep);
}

void Rigidbody::applyForce(const Vector3 &force, const Vector3 &pos)
{
    m_body->applyForce(VEC3_TO_BTVEC3(force), VEC3_TO_BTVEC3(pos));
}

void Rigidbody::applyTorque(const Vector3 &torque)
{
    m_body->applyTorque(VEC3_TO_BTVEC3(torque));
}

void Rigidbody::setAngularVelocity(const Vector3 &value)
{
    m_body->setAngularVelocity(VEC3_TO_BTVEC3(value));
}

Vector3 Rigidbody::getAngularVelocity() const
{
    return BTVEC3_TO_VEC3(m_body->getAngularVelocity());
}

void Rigidbody::setLinearVelocity(const Vector3 &value)
{
    m_body->setLinearVelocity(VEC3_TO_BTVEC3(value));
}

Vector3 Rigidbody::getLinearVelocity() const
{
    return BTVEC3_TO_VEC3(m_body->getLinearVelocity());
}

void Rigidbody::setPosition(const Vector3 &position)
{
    btTransform transform = m_body->getWorldTransform();

    transform.setOrigin(VEC3_TO_BTVEC3(position));

    m_body->setWorldTransform(transform);
}

Vector3 Rigidbody::getPosition() const
{
    btTransform transform = m_body->getWorldTransform();

    return BTVEC3_TO_VEC3(transform.getOrigin());
}

void Rigidbody::setRotation(const Quaternion &rotation)
{
    btTransform transform = m_body->getWorldTransform();

    transform.setRotation(QUAT_TO_BTQUAT(rotation));

    m_body->setWorldTransform(transform);
}

Quaternion Rigidbody::getRotation() const
{
    btTransform transform = m_body->getWorldTransform();

    return BTQUAT_TO_QUAT(transform.getOrigin());
}



