#pragma once

#include <memory>
#include <mutex>

#include <bullet/btBulletDynamicsCommon.h>

#include <gr-math/gmath.hpp>

#include "gr-base/ecs/default.hpp"

#include "default.hpp"

#define VEC3_TO_BTVEC3(vec) btVector3(vec.x, vec.y, vec.z)
#define QUAT_TO_BTQUAT(q) btQuaternion(q.x, q.y, q.z, q.w)

class Hit {
public:
    GameObject *collisionObject;

    Vector3 hitNormal;

    Vector3 hitPoint;
};

class PhysicsScene
{
public:
    static void AddRigidbody(btRigidBody* body);

    static void RemoveRigidbody(btRigidBody* body);

    static void Update(float dt);

    static bool Raycast(const Vector3 &rayStart, const Vector3 &rayEnd, Hit &hit);

    static void ContactTest(btCollisionObject *colObj, btCollisionWorld::ContactResultCallback &resultCallback);

    static Vector3 gravity;

private:
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;

    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

    std::unique_ptr<btBroadphaseInterface> broadphase;

    std::unique_ptr<btCollisionDispatcher> dispatcher;

    std::mutex m_mutex;

    static PhysicsScene &GetInstance();

    PhysicsScene();
    ~PhysicsScene();
};

