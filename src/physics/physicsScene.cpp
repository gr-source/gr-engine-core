#include "gr-base/physics/physicsScene.hpp"

Vector3 PhysicsScene::gravity = {0.0f, -9.8f, 0.0f};

void PhysicsScene::AddRigidbody(btRigidBody* body) {
    GetInstance().dynamicsWorld->addRigidBody(body);
}

void PhysicsScene::RemoveRigidbody(btRigidBody* body) {
    GetInstance().dynamicsWorld->removeRigidBody(body);
}

void PhysicsScene::Update(float dt)
{
    GetInstance().dynamicsWorld->stepSimulation(dt);
}

bool PhysicsScene::Raycast(const Vector3 &rayStart, const Vector3 &rayEnd, Hit &hit)
{
    btCollisionWorld::ClosestRayResultCallback rayCallback({rayStart.x, rayStart.y, rayStart.z}, {rayEnd.x, rayEnd.y, rayEnd.z});
    {
        GetInstance().dynamicsWorld->rayTest({rayStart.x, rayStart.y, rayStart.z}, {rayEnd.x, rayEnd.y, rayEnd.z}, rayCallback);
    }

    hit.collisionObject = nullptr;
    if (rayCallback.m_collisionObject)
        hit.collisionObject = reinterpret_cast<GameObject *>(rayCallback.m_collisionObject->getUserPointer());

    hit.hitNormal = {rayCallback.m_hitNormalWorld.getX(), rayCallback.m_hitNormalWorld.getY(), rayCallback.m_hitNormalWorld.getZ()};
    hit.hitPoint = {rayCallback.m_hitPointWorld.getX(), rayCallback.m_hitPointWorld.getY(), rayCallback.m_hitPointWorld.getZ()};

    return rayCallback.hasHit();
}

void PhysicsScene::ContactTest(btCollisionObject *colObj, btCollisionWorld::ContactResultCallback &resultCallback) {
    return GetInstance().dynamicsWorld->contactTest(colObj, resultCallback);
}

btDiscreteDynamicsWorld* PhysicsScene::GetWorld()
{
    return GetInstance().dynamicsWorld.get();
}



// private
PhysicsScene &PhysicsScene::GetInstance()
{
    static PhysicsScene instance;
    return instance;
}

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
PhysicsScene::PhysicsScene() : broadphase(std::make_unique<btDbvtBroadphase>()), collisionConfiguration(std::make_unique<btDefaultCollisionConfiguration>()), dispatcher(std::make_unique<btCollisionDispatcher>(&(*collisionConfiguration))), solver(std::make_unique<btSequentialImpulseConstraintSolver>())
{
    dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(&(*dispatcher), &(*broadphase), &(*solver), &(*collisionConfiguration));
    dynamicsWorld->setGravity(VEC3_TO_BTVEC3(gravity));

    btGhostPairCallback* ghostCallback = new btGhostPairCallback();
    broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(
        ghostCallback
    );
}

PhysicsScene::~PhysicsScene()
{}

