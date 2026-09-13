#include "gr-base/system/physicsSystem.hpp"

#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btTriangleMesh.h"
#include "LinearMath/btDefaultMotionState.h"

#include "gr-base/physics/kinematicCharacterController.hpp"

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
    dispatcher(nullptr),
    ghostCallback(nullptr)
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

    btGhostPairCallback* ghostCallback = new btGhostPairCallback();
    broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(
        ghostCallback
    );

    // scene->world.AddOnCreate<CapsuleCollider>(&PhysicsSystem::onCreateShape, this);
    // scene->world.AddOnCreate<BoxCollider>(&PhysicsSystem::onCreateShape, this);
    // scene->world.AddOnCreate<Rigidbody>(&PhysicsSystem::onCreateRigidbody, this);
    // scene->world.AddOnCreate<CharacterController>(&PhysicsSystem::onCreateCharacter, this);
}

void PhysicsSystem::OnGUI()
{
}

void PhysicsSystem::Update()
{
    dynamicsWorld->stepSimulation(Clock::deltaTime);

    SyncTransforms(&scene->world);
}

void PhysicsSystem::SyncTransforms(engine::ecs::World* world)
{
    world->Query<TransformLocal, Rigidbody>(
            [&](
                size_t count,
                Entity* entities,
                TransformLocal* transforms,
                const Rigidbody* bodies
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    auto& t = transforms[i];
                    auto& rb = bodies[i];

                    auto body = rb.body;
                    if (body == nullptr || !body->isActive())
                        continue;

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

                    scene->MarkChanged<TransformWorld>(entities[i]);
                }
            }
        );

    world->Query<TransformLocal, CharacterController>(
            [&](
                size_t count,
                Entity* entities,
                TransformLocal* transforms,
                const CharacterController* controllers
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    auto& t = transforms[i];
                    auto& c = controllers[i];

                    auto ghostObject = c.ghostObject;

                    btTransform tr = ghostObject->getWorldTransform();

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

                    scene->MarkChanged<TransformLocal>(entities[i]);
                }
            }
        );
}

void PhysicsSystem::onCreateShape(engine::ecs::World* world, Entity entity)
{
    /*
    auto& transform = world->GetComponent<TransformLocal>(entity);

    btCollisionShape* shape = nullptr;
    if (world->HasComponent<BoxCollider>(entity))
        shape = 
            new btBoxShape(
                btVector3(
                    transform.scale.x,
                    transform.scale.y,
                    transform.scale.z
                )
            );
    else if (world->HasComponent<CapsuleCollider>(entity))
        shape =
            new btCapsuleShape(0.5f, 1.8f);

    if (shape == nullptr)
        return;

    world->AddComponent<PhysicsShape>(
            entity,
            shape
        );
    */
}

void PhysicsSystem::onCreateRigidbody(engine::ecs::World* world, Entity entity)
{
    if (!world->HasComponent<PhysicsShape, Rigidbody>(entity))
        return;

    auto&& [transform, physicsShape, rb] = world->GetComponents<TransformLocal, PhysicsShape, Rigidbody>(entity);

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

void PhysicsSystem::onCreateCharacter(engine::ecs::World* world, Entity entity)
{
    /*
    if (!world->HasComponent<PhysicsShape>(entity))
        world->AddComponent<CapsuleCollider>(entity);

    auto& physicsShape =
        world->GetComponent<PhysicsShape>(entity);

    auto& transform =
        world->GetComponent<TransformLocal>(entity);

    auto& characterController =
        world->GetComponent<CharacterController>(entity);

    auto ghostObject = new btPairCachingGhostObject();
    ghostObject->setCollisionShape(physicsShape.shape);
    ghostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

    btTransform start(
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
    );

    ghostObject->setWorldTransform(start);

    auto character =
      new KinematicCharacterController(
          ghostObject,
          (btConvexShape *)physicsShape.shape,
          0.35f,
          btVector3(0.0f, 1.0f, 0.0f)
      );

    character->setGravity(btVector3(0.0f, -9.8f, 0.0f));
    character->setMaxSlope(btRadians(44.0f));
    character->setFallSpeed(55.0f);
    character->setJumpSpeed(10.0f);

    characterController.ghostObject = ghostObject;
    characterController.character = character;

    AddCollisionObject(ghostObject, character);
    */
}

