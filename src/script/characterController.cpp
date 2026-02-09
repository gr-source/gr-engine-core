#include "gr-base/core/clock.hpp"
#include "gr-base/script/transform.hpp"
#include <gr-base/script/characterController.hpp>

#include <gr-base/physics/physicsScene.hpp>

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#include <gr-base/core/gameObject.hpp>

CharacterController::CharacterController() : m_capsule(nullptr), m_ghost(nullptr), m_character(nullptr)
{}

CharacterController::~CharacterController()
{
    auto world = PhysicsScene::GetWorld();
    if (!world) return;

    if (m_character)
        world->removeAction(m_character);

    if (m_ghost)
        world->removeCollisionObject(m_ghost);

    if (m_character)
        delete m_character;

    if (m_ghost)
        delete m_ghost;

    if (m_capsule)
        delete m_capsule;

    m_character = nullptr;
    m_ghost = nullptr;
    m_capsule = nullptr;
}

void CharacterController::Start()
{
    m_capsule =
        new btCapsuleShape(0.5f, 1.8f);

    m_ghost = new btPairCachingGhostObject();
    m_ghost->setCollisionShape(m_capsule);
    m_ghost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);


    auto transform = GetEntity()->getComponent<Transform>();

    btTransform start(
			QUAT_TO_BTQUAT(transform->getRotation()),
			VEC3_TO_BTVEC3(transform->getPosition())
		);

    m_ghost->setWorldTransform(start);

    m_character =
        new btKinematicCharacterController(
            m_ghost,
            m_capsule,
            0.35f
        );
    m_character->setGravity({PhysicsScene::gravity.x, PhysicsScene::gravity.y, PhysicsScene::gravity.z});
    m_character->setMaxSlope(btRadians(45.0f));
    m_character->setFallSpeed(55.0f);
    m_character->setJumpSpeed(10.0f);

    auto dynamicsWorld = PhysicsScene::GetWorld();
    dynamicsWorld->addCollisionObject(
        m_ghost,
        btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::StaticFilter |
        btBroadphaseProxy::DefaultFilter
    );

    dynamicsWorld->addAction(m_character);
}

void CharacterController::Update()
{
    const btTransform& worldTrans =
        m_ghost->getWorldTransform();

    btVector3 pos = worldTrans.getOrigin();
    // btQuaternion rot = trans.getRotation();

    auto transform = GetEntity()->getComponent<Transform>();
    transform->setLocalPosition(*reinterpret_cast<const Vector3*>(&pos));

    // transform->setLocalRotation(*reinterpret_cast<const Quaternion*>(&rot));
}

void CharacterController::SetWalkingDirection(const Vector3& direction)
{
    m_character->setWalkDirection(*reinterpret_cast<const btVector3*>(&direction));
}


