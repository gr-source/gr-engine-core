#include "gr-base/system/default.hpp"

#include "gr-base/ecs/systemRegistry.hpp"
#include "gr-base/system/camerasystem.hpp"
// #include "gr-base/system/characterControllerSystem.hpp"
#include "gr-base/system/physicsSystem.hpp"
#include "gr-base/system/renderSystem.hpp"
#include "gr-base/system/transformsystem.hpp"

#define REGISTER_SYSTEM(K, ...) \
    do{ \
        SystemRegistry::Register<K __VA_OPT__(,) __VA_ARGS__>(); \
    } while (0)

void RegistryDefaultSystem()
{
    // REGISTER_SYSTEM(CharacterControllerSystem);

    REGISTER_SYSTEM(PhysicsSystem);

    REGISTER_SYSTEM(TransformSystem,
            PhysicsSystem);

    REGISTER_SYSTEM(CameraSystem,
        TransformSystem);

    REGISTER_SYSTEM(RenderSystem,
            TransformSystem);
}


