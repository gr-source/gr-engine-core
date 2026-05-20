#pragma once

#include <memory>
#include <mutex>

#include <bullet/btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include <gr-math/gmath.hpp>
#include <vector>

#include "gr-base/core/default.hpp"

#include "default.hpp"

#include "gr-base/ecs/default.hpp"

#define VEC3_TO_BTVEC3(vec) btVector3(vec.x, vec.y, vec.z)
#define QUAT_TO_BTQUAT(q) btQuaternion(q.x, q.y, q.z, q.w)

class Hit {
public:
    Entity collisionObject;

    Vector3 hitNormal;

    Vector3 hitPoint;
};

struct PendingRemoval
{
    std::shared_ptr<btRigidBody> body;
    std::shared_ptr<btCollisionShape> shape;
};

class PhysicsScene
{
public:
    static void AddRigidbody(btRigidBody* body);

    static void RemoveRigidbody(PendingRemoval body);

    static void Update(float dt);

    static bool Raycast(const Vector3 &rayStart, const Vector3 &rayEnd, Hit &hit);

    static void ContactTest(btCollisionObject *colObj, btCollisionWorld::ContactResultCallback &resultCallback);

    static btDiscreteDynamicsWorld* GetWorld();

    static Vector3 gravity;

private:
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;

    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;

    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

    std::unique_ptr<btBroadphaseInterface> broadphase;

    std::unique_ptr<btCollisionDispatcher> dispatcher;

    std::vector<PendingRemoval> m_bodyToRemove;

    std::mutex m_mutex;

    static PhysicsScene &GetInstance();

    PhysicsScene();
    ~PhysicsScene();

    void addRigidbody(btRigidBody* body);

    void removeRigidbody(PendingRemoval body);

    void update(float dt);
};

