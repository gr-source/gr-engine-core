#include "physics/physicsScene.hpp"

Vector3 PhysicsScene::gravity = {0.0f, -9.8f, 0.0f};

std::unique_ptr<btDefaultCollisionConfiguration> PhysicsScene::collisionConfiguration = nullptr;

std::unique_ptr<btSequentialImpulseConstraintSolver> PhysicsScene::solver = nullptr;

std::unique_ptr<btDiscreteDynamicsWorld> PhysicsScene::dynamicsWorld = nullptr;

std::unique_ptr<btBroadphaseInterface> PhysicsScene::broadphase = nullptr;

std::unique_ptr<btCollisionDispatcher> PhysicsScene::dispatcher = nullptr;

std::mutex PhysicsScene::m_mutex;

void PhysicsScene::Initialize() {
    std::unique_lock<std::mutex> lock(m_mutex);

    broadphase             = std::make_unique<btDbvtBroadphase>();

    collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();

    dispatcher             = std::make_unique<btCollisionDispatcher>(&(*collisionConfiguration));

    solver                 = std::make_unique<btSequentialImpulseConstraintSolver>();

    dynamicsWorld          = std::make_unique<btDiscreteDynamicsWorld>(&(*dispatcher), &(*broadphase), &(*solver), &(*collisionConfiguration));

    dynamicsWorld->setGravity(VEC3_TO_BTVEC3(gravity));
}

void PhysicsScene::Shutdown() {
}

void PhysicsScene::AddRigidbody(btRigidBody* body) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dynamicsWorld->addRigidBody(body);
}

void PhysicsScene::RemoveRigidbody(btRigidBody* body) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dynamicsWorld->removeRigidBody(body);
}

void PhysicsScene::Update(float dt) {
    std::lock_guard<std::mutex> lock(m_mutex);
    dynamicsWorld->stepSimulation(dt);
}

bool PhysicsScene::Raycast(const Vector3 &rayStart, const Vector3 &rayEnd, Hit &hit) {
    btCollisionWorld::ClosestRayResultCallback rayCallback({rayStart.x, rayStart.y, rayStart.z}, {rayEnd.x, rayEnd.y, rayEnd.z});
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        dynamicsWorld->rayTest({rayStart.x, rayStart.y, rayStart.z}, {rayEnd.x, rayEnd.y, rayEnd.z}, rayCallback);
    }

    hit.collisionObject = nullptr;
    if (rayCallback.m_collisionObject)
        hit.collisionObject = reinterpret_cast<GameObject *>(rayCallback.m_collisionObject->getUserPointer());

    hit.hitNormal = {rayCallback.m_hitNormalWorld.getX(), rayCallback.m_hitNormalWorld.getY(), rayCallback.m_hitNormalWorld.getZ()};
    hit.hitPoint = {rayCallback.m_hitPointWorld.getX(), rayCallback.m_hitPointWorld.getY(), rayCallback.m_hitPointWorld.getZ()};

    return rayCallback.hasHit();
}

void PhysicsScene::contactTest(btCollisionObject *colObj, btCollisionWorld::ContactResultCallback &resultCallback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return dynamicsWorld->contactTest(colObj, resultCallback);
}
