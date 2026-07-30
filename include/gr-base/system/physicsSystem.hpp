#pragma once

#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"

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

#include <gr-math/gmath.hpp>

#include <memory>

class PhysicsSystem : public ISystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem() override;

    SparseID CreateMotionState(const Vector3& position = type_traits<Vector3>::zero, const Quaternion& rotation = type_traits<Quaternion>::identity);

    btDefaultMotionState* GetMotionState(SparseID id);

    SparseID CreateRigidbody(const btRigidBody::btRigidBodyConstructionInfo& info);

    btRigidBody* GetRigidbody(SparseID id);

    void AddBody(SparseID id);

    void RemoveBody(SparseID id);

    void DestroyBody(SparseID id);

    SparseID CreateTriangleMeshShape(const Mesh* mesh);

    SparseID CreateBoxShape(const Vector3& boxHalfExtents);

    btCollisionShape* GetShape(SparseID id);

private:
    btDefaultCollisionConfiguration* collisionConfiguration;

    btSequentialImpulseConstraintSolver* solver;

    btDiscreteDynamicsWorld* dynamicsWorld;

    btBroadphaseInterface* broadphase;

    btCollisionDispatcher* dispatcher;

    SparseSet<btDefaultMotionState> motionsSparseset;

    SparseID nextMotionStateID = 0;

    SparseSet<btRigidBody> rigidBodySparseset;

    SparseID nextBodyID = 0;

    std::vector<btCollisionShape*> shapeList;

    SparseID nextTriangleMeshShapeID = 0;

    void Init() override;

    void Update(Scene* scene) override;

    void createShape(engine::ecs::world* world);

    void createBody(engine::ecs::world* world);

    void syncPhysicsToTransform(Scene* scene, engine::ecs::world* world);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<PhysicsSystem> =
    TypeCategory::System;


