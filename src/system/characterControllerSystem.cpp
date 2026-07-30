#include "gr-base/system/characterControllerSystem.hpp"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/characterController.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/system/physicsSystem.hpp"

void CharacterControllerSystem::Update()
{
    /*
    auto p = scene->GetSystem<PhysicsSystem>();
    if (p == nullptr)
        return;
    scene->world->Query<Transform, CharacterController>(
            [&](
                size_t count,
                Entity*,
                Transform* transforms,
                CharacterController* controllers
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    auto& transform = transforms[i];
                    auto& controller = controllers[i];

                    if (controller.character == nullptr)
                    {
                        controller.capsuleShape = new btCapsuleShape(0.5f, 1.8f);

                        controller.ghostObject = new btPairCachingGhostObject();

                        btTransform startTransform;
                        startTransform.setIdentity();
                        startTransform.setOrigin(btVector3(transform.position.x, transform.position.y, transform.position.z));
                        startTransform.setRotation(btQuaternion(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w));

                        controller.ghostObject->setWorldTransform(startTransform);
                        controller.ghostObject->setCollisionShape(controller.capsuleShape);

                        controller.ghostObject->setCollisionFlags(
                            btCollisionObject::CF_CHARACTER_OBJECT);

                        controller.character = new btKinematicCharacterController(
                            controller.ghostObject,
                            controller.capsuleShape,
                            0.35f
                        );

                        p->AddCollisionObject(controller.ghostObject, controller.character);
                    }

                    btTransform tr = controller.ghostObject->getWorldTransform();

                    auto p = tr.getOrigin();
                    auto q = tr.getRotation();

                    transform.position = {
                        p.x(),
                        p.y(),
                        p.z()
                    };

                    transform.rotation = Quaternion{
                        q.w(),
                        q.x(),
                        q.y(),
                        q.z()
                    };
                }
            }
        );
        */
}


