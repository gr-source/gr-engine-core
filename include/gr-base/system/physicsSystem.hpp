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
#include <stack>

class PhysicsSystem : public ISystem
{
public:
    static btCollisionShape* BuildMeshShape(const Mesh* mesh);

    PhysicsSystem();
    ~PhysicsSystem() override;

    SparseID CreateMotionState(const Vector3& position = type_traits<Vector3>::zero, const Quaternion& rotation = type_traits<Quaternion>::identity);

    void DestroyMotion(SparseID id);

    btMotionState* GetMotionState(SparseID id);

    SparseID CreateRigidbody(const btRigidBody::btRigidBodyConstructionInfo& info);

    btRigidBody* GetRigidbody(SparseID id);

    void DestroyBody(SparseID id);

    void AddBody(SparseID id);

    void RemoveBody(SparseID id);

    SparseID CreateShapeID();

    void DestroyShape(SparseID id);

    void SetShape(SparseID id, btCollisionShape* shape);

    SparseID CreateBoxShape(const Vector3& boxHalfExtents);

    btCollisionShape* GetShape(SparseID id);

private:
    btDefaultCollisionConfiguration* collisionConfiguration;

    btSequentialImpulseConstraintSolver* solver;

    btDiscreteDynamicsWorld* dynamicsWorld;

    btBroadphaseInterface* broadphase;

    btCollisionDispatcher* dispatcher;

    std::vector<btMotionState*> motionsStateList;

    std::stack<SparseID> motionFreeIDs;

    std::vector<btRigidBody*> bodies;

    std::stack<SparseID> bodyFreeIDs;

    std::vector<btCollisionShape*> shapes;

    std::stack<SparseID> shapeFreeIDs;

    void Init() override;

    void OnGUI() override;

    void Update() override;

    void createShape(engine::ecs::world* world);

    void createBody(engine::ecs::world* world);

    void syncPhysicsToTransform(engine::ecs::world* world);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<PhysicsSystem> =
    TypeCategory::System;


