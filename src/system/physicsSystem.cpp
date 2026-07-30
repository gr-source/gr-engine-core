#include "gr-base/system/physicsSystem.hpp"

#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "LinearMath/btDefaultMotionState.h"
#include "gr-base/core/geometry/mesh.hpp"
#include "gr-base/core/scene.hpp"
#include "gr-base/core/clock.hpp"
#include "gr-base/script/characterController.hpp"
#include "gr-base/script/collider.hpp"
#include "gr-base/script/transform.hpp"
#include "imgui.h"

btCollisionShape* PhysicsSystem::BuildMeshShape(const Mesh* mesh)
{
    btTriangleMesh* triangleMesh =
        new btTriangleMesh();

    for (size_t i=0;i<mesh->numIndices;i+=3)
    {
        const auto& v0 =
            mesh->vertices[mesh->indices[i + 0]];
        const auto& v1 =
            mesh->vertices[mesh->indices[i + 1]];
        const auto& v2 =
            mesh->vertices[mesh->indices[i + 2]];

        triangleMesh->addTriangle(
                btVector3(
                    v0.position.x,
                    v0.position.y,
                    v0.position.z
                ),
                btVector3(
                    v1.position.x,
                    v1.position.y,
                    v1.position.z
                ),
                btVector3(
                    v2.position.x,
                    v2.position.y,
                    v2.position.z
                )
            );
    }

    return new btBvhTriangleMeshShape(triangleMesh, true);
}

PhysicsSystem::PhysicsSystem() :
    collisionConfiguration(nullptr),
    solver(nullptr),
    dynamicsWorld(nullptr),
    broadphase(nullptr),
    dispatcher(nullptr)
{}

PhysicsSystem::~PhysicsSystem()
{}

void PhysicsSystem::AddBody(btRigidBody* body)
{
    dynamicsWorld->addRigidBody(body);
}

void PhysicsSystem::RemoveBody(btRigidBody* body)
{
    dynamicsWorld->removeRigidBody(body);
}

void PhysicsSystem::AddCollisionObject(btCollisionObject* collisionObject, btActionInterface* actionInterface)
{
    dynamicsWorld->addCollisionObject(
        collisionObject,
        btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::StaticFilter |
        btBroadphaseProxy::DefaultFilter);

    dynamicsWorld->addAction(actionInterface);
}

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

    auto world = scene->world;

    world->AddOnCreate<BoxCollider>(&PhysicsSystem::onCreateShape, this);
    world->AddOnCreate<Rigidbody>(&PhysicsSystem::onCreateRigidbody, this);
}

void PhysicsSystem::OnGUI()
{
}

void PhysicsSystem::Update()
{
    dynamicsWorld->stepSimulation(Clock::deltaTime);

    SyncTransforms(scene->world);
}

void PhysicsSystem::onCreateShape(engine::ecs::world* world, Entity entity)
{
    auto transform = world->GetComponent<Transform>(entity);

    world->AddComponent<PhysicsShape>(
            entity,
            new btBoxShape(
                btVector3(
                    transform->scale.x,
                    transform->scale.y,
                    transform->scale.z
                )
            )
        );
}

void PhysicsSystem::onCreateRigidbody(engine::ecs::world* world, Entity entity)
{
    if (!world->HasComponent<Transform, PhysicsShape, Rigidbody>(entity))
        return;

    auto&& [transform, physicsShape, rb] = world->GetComponents<Transform, PhysicsShape, Rigidbody>(entity);

    auto shape =
        physicsShape->shape;

    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
    shape->calculateLocalInertia(rb->mass, fallInertia);

    rb->motionState = new btDefaultMotionState(
            btTransform(
                btQuaternion(
                    transform->rotation.x,
                    transform->rotation.y,
                    transform->rotation.z,
                    transform->rotation.w
                ),
                btVector3(
                    transform->position.x,
                    transform->position.y,
                    transform->position.z
                )
            )
        );

    btRigidBody::btRigidBodyConstructionInfo info(
                rb->mass,
                rb->motionState,
                shape,
                fallInertia
            );

    rb->body = new btRigidBody(info);

    dynamicsWorld->addRigidBody(rb->body);
}

void PhysicsSystem::SyncTransforms(engine::ecs::world* world)
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

                    auto body = rb.body;

                    btTransform tr;
                    body->getMotionState()->getWorldTransform(tr);

                    auto p = tr.getOrigin();
                    auto q = tr.getRotation();

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
                }
            }
        );

    world->Query<Transform, CharacterController>(
            [&](
                size_t count,
                Entity* entities,
                Transform* transforms,
                CharacterController* controllers
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    auto& t = transforms[i];
                    auto& c = controllers[i];

                    btTransform tr = c.ghostObject->getWorldTransform();

                    auto p = tr.getOrigin();
                    auto q = tr.getRotation();

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
                }
            }
        );
}


