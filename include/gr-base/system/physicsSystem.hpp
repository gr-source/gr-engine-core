#pragma once

#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"

#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"

#include "LinearMath/btDefaultMotionState.h"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/system.hpp"
#include "gr-base/ecs/world.hpp"
#include "gr-base/pool/sparseSet.hpp"
#include "gr-base/script/collider.hpp"

#include <gr-math/gmath.hpp>

#include <memory>
#include <stack>

class PhysicsSystem : public ISystemImpl<Read<>, Write<>>
{
public:
    static btCollisionShape* BuildMeshShape(const Mesh* mesh);

    PhysicsSystem();
    ~PhysicsSystem() override;

    void AddBody(btRigidBody* body);

    void RemoveBody(btRigidBody* body);

    void AddCollisionObject(btCollisionObject* collisionObject, btActionInterface* actionInterface);

private:
    btDefaultCollisionConfiguration* collisionConfiguration;

    btSequentialImpulseConstraintSolver* solver;

    btDiscreteDynamicsWorld* dynamicsWorld;

    btBroadphaseInterface* broadphase;

    btCollisionDispatcher* dispatcher;

    btGhostPairCallback* ghostCallback;

    void Init() override;

    void OnGUI() override;

    void Update() override;

    void SyncInput(engine::ecs::world* world);

    void SyncTransforms(engine::ecs::world* world);

    void onCreateShape(engine::ecs::world* world, Entity entity);

    void onCreateRigidbody(engine::ecs::world* world, Entity entity);

    void onCreateCharacter(engine::ecs::world* world, Entity entity);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<PhysicsSystem> =
    TypeCategory::System;


