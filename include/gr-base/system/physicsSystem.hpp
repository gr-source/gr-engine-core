#pragma once

#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"

#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"

#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h"

#include "gr-base/core/system.hpp"
#include "gr-base/ecs/world.hpp"
#include "gr-base/pool/sparseSet.hpp"

#include <memory>

class PhysicsSystem : public ISystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem() override;

private:
    btDefaultCollisionConfiguration* collisionConfiguration;

    btSequentialImpulseConstraintSolver* solver;

    btDiscreteDynamicsWorld* dynamicsWorld;

    btBroadphaseInterface* broadphase;

    btCollisionDispatcher* dispatcher;

    SparseSet<btRigidBody> m_rigidbodies;

    size_t numBodies = 0;

    void Init() override;

    void Update(Scene* scene) override;

    void createShape(engine::ecs::world* world);

    void createBody(engine::ecs::world* world);

    void syncPhysicsToTransform(Scene* scene, engine::ecs::world* world);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<PhysicsSystem> =
    TypeCategory::System;


