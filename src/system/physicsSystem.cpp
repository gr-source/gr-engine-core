#include "gr-base/system/physicsSystem.hpp"

#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "LinearMath/btDefaultMotionState.h"
#include "gr-base/core/geometry/mesh.hpp"
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

SparseID PhysicsSystem::CreateMotionState(const Vector3& position, const Quaternion& rotation)
{
    SparseID id = nextMotionStateID++;

    motionsSparseset.insert(
            id,
            btTransform(
                btQuaternion(
                    rotation.x,
                    rotation.y,
                    rotation.z,
                    rotation.w
                ),
                btVector3(
                    position.x,
                    position.y,
                    position.z
                )
            )
        );

    return id;
}

btDefaultMotionState* PhysicsSystem::GetMotionState(SparseID id)
{
    return motionsSparseset.get(id);
}

SparseID PhysicsSystem::CreateRigidbody(const btRigidBody::btRigidBodyConstructionInfo& info)
{
    SparseID id = nextBodyID++;

    rigidBodySparseset.insert(id, info);

    return id;
}

btRigidBody* PhysicsSystem::GetRigidbody(SparseID id)
{
    return rigidBodySparseset.get(id);
}

void PhysicsSystem::AddBody(SparseID id)
{
    auto body = GetRigidbody(id);
    if (body == nullptr)
        return;

    dynamicsWorld->addRigidBody(body);
}

void PhysicsSystem::RemoveBody(SparseID id)
{
    auto body = GetRigidbody(id);
    if (body == nullptr)
        return;

    dynamicsWorld->removeRigidBody(body);
}

void PhysicsSystem::DestroyBody(SparseID id)
{
    rigidBodySparseset.destroy(id);
}

SparseID PhysicsSystem::CreateTriangleMeshShape(const Mesh* mesh)
{
    SparseID id = nextTriangleMeshShapeID++;

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

    auto shape = new btBvhTriangleMeshShape(triangleMesh, true);
    shape->getMeshInterface();

    if (id >= shapeList.size())
        shapeList.resize(id + 1, nullptr);

    shapeList[id] = shape;

    return id;
}

SparseID PhysicsSystem::CreateBoxShape(const Vector3& boxHalfExtents)
{
    SparseID id = nextTriangleMeshShapeID++;

    auto shape = new btBoxShape(
            btVector3(
                boxHalfExtents.x,
                boxHalfExtents.y,
                boxHalfExtents.z
            )
        );

    if (id >= shapeList.size())
        shapeList.resize(id + 1, nullptr);

    shapeList[id] = shape;

    return id;
}

btCollisionShape* PhysicsSystem::GetShape(SparseID id)
{
    if (id >= shapeList.size())
        return nullptr;
    return shapeList[id];
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
                            CreateBoxShape(
                                    transform.scale
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
                    if (bodies[i].body != INVALID32)
                        continue;

                    auto& transform = transforms[i];
                    auto& physicsShape = shapes[i];
                    auto& rb = bodies[i];

                    auto shape =
                        GetShape(physicsShape.shapeID);

                    btVector3 fallInertia(0.0f, 0.0f, 0.0f);
                    shape->calculateLocalInertia(rb.mass, fallInertia);

                    rb.motion =
                        CreateMotionState(
                                        transform.position,
                                        transform.rotation
                                    );

                    btRigidBody::btRigidBodyConstructionInfo info(
                                rb.mass,
                                GetMotionState(rb.motion),
                                shape,
                                fallInertia
                            );

                    rb.body = CreateRigidbody(info);

                    AddBody(rb.body);
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

                    btRigidBody* body = GetRigidbody(rb.body);

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


