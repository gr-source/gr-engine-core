#include "gr-base/system/characterInputSystem.hpp"

#include "gr-base/physics/kinematicCharacterController.hpp"

#include "gr-base/core/scene.hpp"

void CharacterInputSystem::Update()
{
    scene->world->Query<CharacterController, CharacterInput>(
            [&](
                size_t count,
                Entity* entities,
                CharacterController* controllers,
                const CharacterInput* inputs
            )
            {
                for (size_t i=0;i<count;i++)
                {
                    auto& input = inputs[i];
                    auto& controller = controllers[i];

                    controller.character->setWalkDirection(
                            btVector3(
                                input.move.x,
                                input.move.y,
                                input.move.z
                            )
                        );

                    if (controller.character->canJump() && input.jump)
                    {
                        controller.character->jump();
                    }
                }
            }
        );
}

