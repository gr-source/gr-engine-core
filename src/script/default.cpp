#include "gr-base/script/default.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/type_id.hpp"

#include "gr-base/script/characterController.hpp"
#include "gr-base/script/collider.hpp"
#include "gr-base/script/relationship.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"
#include "gr-base/script/name.hpp"

#define REGISTER_COMPONENT(K, b) \
        do \
        { \
            ComponentRegistry::Register<K>(b); \
            GetComponentID<K>::id = \
                TypeId<IdCategory::COMPONENT>::GetID<K>(); \
        } while (0)

void RegistryDefaultComponent()
{
    logger::debug("[ComponentRegistry::Registry] Carregando script padrões...");

    REGISTER_COMPONENT(Relationship, false);

    REGISTER_COMPONENT(MeshRender, false);
    REGISTER_COMPONENT(RenderBounds, false);

    REGISTER_COMPONENT(MeshFilter, false);
    
    REGISTER_COMPONENT(Static, true);

    REGISTER_COMPONENT(TransformLocal, true);
    REGISTER_COMPONENT(TransformWorld, true);

    REGISTER_COMPONENT(CameraRenderResult, false);
    REGISTER_COMPONENT(CameraData, false);
    REGISTER_COMPONENT(Camera, true);
    REGISTER_COMPONENT(Light, true);
    REGISTER_COMPONENT(Name, false);

    REGISTER_COMPONENT(PhysicsShape, false);

    // REGISTER_COMPONENT(CapsuleCollider, false);
    // REGISTER_COMPONENT(BoxCollider, false);
    
    REGISTER_COMPONENT(Rigidbody, false);

    REGISTER_COMPONENT(CharacterController, false);
    REGISTER_COMPONENT(CharacterState, false);
    REGISTER_COMPONENT(CharacterInput, false);
}

#undef REGISTER_COMPONENT

