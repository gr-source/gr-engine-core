#include "gr-base/system/characterControllerSystem.hpp"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "gr-base/physics/kinematicCharacterController.hpp"

#include "gr-base/core/scene.hpp"

#include "gr-base/script/characterController.hpp"
#include "gr-base/script/transform.hpp"

void CharacterControllerSystem::Update()
{
    scene->world->Query<TransformLocal, CharacterController>(
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

                    // c.canJump = c.character->canJump();

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


