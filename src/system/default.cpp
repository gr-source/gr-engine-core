#include "gr-base/system/default.hpp"

#include "gr-base/ecs/systemRegistry.hpp"
#include "gr-base/system/camerasystem.hpp"
// #include "gr-base/system/characterControllerSystem.hpp"
#include "gr-base/system/characterControllerSystem.hpp"
#include "gr-base/system/characterInputSystem.hpp"
#include "gr-base/system/lightSystem.hpp"
#include "gr-base/system/physicsSystem.hpp"
#include "gr-base/system/renderSystem.hpp"
#include "gr-base/system/transformsystem.hpp"

#define REGISTER_SYSTEM(K, ...) \
    do{ \
        SystemRegistry::Register<K __VA_OPT__(,) __VA_ARGS__>(); \
        GetSystemID<K>::id = \
            TypeId<IdCategory::SYSTEM>::GetID<K>(); \
    } while (0)

void RegistryDefaultSystem()
{
    REGISTER_SYSTEM(LightSystem);

    REGISTER_SYSTEM(PhysicsSystem);

    REGISTER_SYSTEM(CharacterInputSystem);

    REGISTER_SYSTEM(TransformSystem,
            PhysicsSystem);

    REGISTER_SYSTEM(CameraSystem,
        TransformSystem);

    REGISTER_SYSTEM(RenderSystem,
            TransformSystem);
}


