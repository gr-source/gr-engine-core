#include "gr-base/system/default.hpp"

#include "gr-base/ecs/systemRegistry.hpp"
#include "gr-base/system/camerasystem.hpp"
#include "gr-base/system/physicsSystem.hpp"
#include "gr-base/system/renderSystem.hpp"
#include "gr-base/system/transformsystem.hpp"

#define REGISTER_SYSTEM(K, ...) \
    do{ \
        SystemRegistry::Register<K __VA_OPT__(,) __VA_ARGS__>(); \
    } while (0)

void RegistryDefaultSystem()
{
    REGISTER_SYSTEM(TransformSystem);
    REGISTER_SYSTEM(CameraSystem);

    REGISTER_SYSTEM(PhysicsSystem, TransformSystem);

    REGISTER_SYSTEM(RenderSystem, PhysicsSystem, TransformSystem);
}


