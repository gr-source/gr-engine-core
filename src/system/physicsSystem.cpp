#include "gr-base/system/physicsSystem.hpp"

#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "LinearMath/btDefaultMotionState.h"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/clock.hpp"
#include "gr-base/script/collider.hpp"
#include "gr-base/script/transform.hpp"

PhysicsSystem::PhysicsSystem() :
    collisionConfiguration(nullptr),
    solver(nullptr),
    dynamicsWorld(nullptr),
    broadphase(nullptr),
    dispatcher(nullptr)
{}

PhysicsSystem::~PhysicsSystem()
{}

// private
void PhysicsSystem::Init()
{
    collisionConfiguration =
        new btDefaultCollisionConfiguration();

    dispatcher =
        new btCollisionDispatcher(collisionConfiguration);

    broadphase =
        new btDbvtBroadphase();

    solver =
        new btSequentialImpulseConstraintSolver();

    dynamicsWorld =
        new btDiscreteDynamicsWorld(
                dispatcher,
                broadphase,
                solver,
                collisionConfiguration
            );

    dynamicsWorld->setGravity(btVector3(0.0f, -9.8f, 0.0f));
}

void PhysicsSystem::Update(Scene* scene)
{
    createShape(scene->world);

    createBody(scene->world);

    dynamicsWorld->stepSimulation(Clock::deltaTime);

    syncPhysicsToTransform(scene, scene->world);
}

void PhysicsSystem::createShape(engine::ecs::world* world)
{
    world->Query<Transform, BoxCollider>(
            [&](
                size_t count,
                Entity* entities,
                Transform* transforms,
                BoxCollider* boxes
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    if (world->HasComponent<PhysicsShape>(entities[i]))
                        continue;

                    auto& transform = transforms[i];

                    world->AddComponent<PhysicsShape>(
                            entities[i],
                            new btBoxShape(
                                btVector3(
                                    transform.scale.x,
                                    transform.scale.y,
                                    transform.scale.z
                                )
                            )
                        );
                }
            }
        );
}

void PhysicsSystem::createBody(engine::ecs::world* world)
{
    world->Query<Transform, Rigidbody, PhysicsShape>(
            [&](
                size_t count,
                Entity* entities,
                Transform* transforms,
                Rigidbody* bodies,
                PhysicsShape* shapes
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    if (bodies[i].body != INVALID_BODY)
                        continue;

                    auto& transform = transforms[i];
                    auto& shape = shapes[i];
                    auto& rb = bodies[i];

                    uint32_t bodyID = numBodies++;

                    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
                    shape.shape->calculateLocalInertia(rb.mass, fallInertia);

                    auto state =
                        new btDefaultMotionState(
                                btTransform(
                                    btQuaternion(
                                        transform.rotation.x,
                                        transform.rotation.y,
                                        transform.rotation.z,
                                        transform.rotation.w
                                    ),
                                    btVector3(
                                        transform.position.x,
                                        transform.position.y,
                                        transform.position.z
                                    )
                                )
                            );

                    btRigidBody::btRigidBodyConstructionInfo info(
                                rb.mass,
                                state,
                                shape.shape,
                                fallInertia
                            );

                    m_rigidbodies.insert(bodyID, info);

                    dynamicsWorld->addRigidBody(m_rigidbodies.get(bodyID));

                    rb.body = bodyID;
                }
            }
        );
}

void PhysicsSystem::syncPhysicsToTransform(Scene* scene, engine::ecs::world* world)
{
    world->Query<Transform, Rigidbody>(
            [&](
                size_t count,
                Entity* entities,
                Transform* transforms,
                Rigidbody* bodies
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    auto& t = transforms[i];
                    auto& rb = bodies[i];

                    btRigidBody* body = m_rigidbodies.get(rb.body);

                    btTransform tr;
                    body->getMotionState()->getWorldTransform(tr);

                    auto p = tr.getOrigin();
                    auto q = tr.getRotation();

                    // std::cout << p.x() << " " << p.y() << " " << p.z()<< std::endl;

                    t.position = {
                        p.x(),
                        p.y(),
                        p.z()
                    };

                    t.rotation = Quaternion{
                        q.w(),
                        q.x(),
                        q.y(),
                        q.z()
                    };

                    scene->MarkChanged<Transform>(entities[i]);

                    /*
                    btTransform tr;
                    tr.setOrigin(btVector3(
                        t.position.x,
                        t.position.y,
                        t.position.z));

                    tr.setRotation(btQuaternion(
                        t.rotation.x,
                        t.rotation.y,
                        t.rotation.z,
                        t.rotation.w));

                    rb.body->setWorldTransform(tr);

                    if (rb.body->getMotionState())
                        rb.body->getMotionState()->setWorldTransform(tr)
                        */
                }
            }
        );
}


