#include "gr-base/script/rigidbody.hpp"

#include "gr-base/script/meshCollider.hpp"
#include "gr-base/script/boxCollider.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/physics/physicsScene.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/file/stream.hpp"

Rigidbody::Rigidbody() : m_collisionShape(nullptr), m_DefaultMotionState(nullptr), m_body(nullptr), m_transform(nullptr), m_mass(0.0f), m_lookRotation(Vector3::one), fMotionThreshold(0.0f), fSweptSphereRadius(0.0f) {}

void Rigidbody::Init() {
    if (entity->hasComponent<MeshCollider>()) {
        m_collisionShape = entity->getComponent<MeshCollider>()->getShape();
    } else if (entity->hasComponent<BoxCollider>()) {
        m_collisionShape = entity->getComponent<BoxCollider>()->getShape();
    }

    if (!m_collisionShape) {
        return Console::LogError("[ Rigidbody ] Shape not found.");
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
    pRigidBodyConstructionInfo.m_friction = 0.5f;
    pRigidBodyConstructionInfo.m_restitution = 0.1f;

    if (m_body) {
        Destroy();
        m_body = nullptr;
    }

    m_body = std::make_shared<btRigidBody>(pRigidBodyConstructionInfo);
	m_body->setAngularFactor(VEC3_TO_BTVEC3(m_lookRotation));
    m_body->setUserPointer(reinterpret_cast<void *>(entity->getGameObject()));

    if (fMotionThreshold >= 0.0f)
        m_body->setCcdMotionThreshold(fMotionThreshold);

    if (fSweptSphereRadius >= 0.0f)
        m_body->setCcdSweptSphereRadius(fSweptSphereRadius);

    PhysicsScene::AddRigidbody(&(*m_body));
}

void Rigidbody::Destroy() {
    if (!m_body) {
        return;
    }

    PhysicsScene::RemoveRigidbody(&(*m_body));
}

void Rigidbody::Update() {
    if (!m_body || !m_transform) {
        return;
    }

    if (getGameObject()->getActive()) {
        if (m_body->getLinearVelocity().length() < 0.01 && m_body->getAngularVelocity().length() < 0.01) {
            m_body->setActivationState(DISABLE_DEACTIVATION);
        } else {
            m_body->forceActivationState(ACTIVE_TAG);
        }
	}

	if (m_body->isActive()) {
		btTransform trans;
		m_body->getMotionState()->getWorldTransform(trans);

		m_transform->setLocalPosition({trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()});

		m_transform->setLocalRotation({trans.getRotation().getW(), trans.getRotation().getX(), trans.getRotation().getY(), trans.getRotation().getZ()});
	}
}

bool Rigidbody::Serialize(Stream *stream) {
    stream->write(&m_mass, sizeof(float));
    stream->write(&m_lookRotation, sizeof(Vector3));
    stream->write(&fMotionThreshold, sizeof(float));
    stream->write(&fSweptSphereRadius, sizeof(float));
    return true;
}

bool Rigidbody::Deserialize(Stream *stream) {
    stream->read(reinterpret_cast<char *>(&m_mass), sizeof(float));
    stream->read(reinterpret_cast<char *>(&m_lookRotation), sizeof(Vector3));
    stream->read(reinterpret_cast<char *>(&fMotionThreshold), sizeof(float));
    stream->read(reinterpret_cast<char *>(&fSweptSphereRadius), sizeof(float));
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
