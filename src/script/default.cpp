#include "gr-base/script/default.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/ecs/componentregistry.hpp"

#include "gr-base/script/characterController.hpp"
#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshCollider.hpp"
#include "gr-base/script/boxCollider.hpp"
#include "gr-base/script/baserender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/rigidbody.hpp"
#include "gr-base/script/collider.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"

#define REGISTER_COMPONENT_WITH_PARENT(k, p) \
    TypeInfo& b##k = ComponentRegistry::Registry<k>(#k, p)

#define CLASS_TO_HASH(k) GetHashByType<k>(#k)

#define REGISTER_COMPONENT(k) REGISTER_COMPONENT_WITH_PARENT(k, (uint64_t)-1)

void RegistryDefaultComponent()
{
    logger::debug("[Editor] Carregando script padrões...");

    REGISTER_COMPONENT(CharacterController);
    REGISTER_COMPONENT(BaseRender);
    REGISTER_COMPONENT(MeshFilter);
    REGISTER_COMPONENT(Transform);
    REGISTER_COMPONENT(Rigidbody);
    REGISTER_COMPONENT(Collider);
    REGISTER_COMPONENT(Camera);
    REGISTER_COMPONENT(Light);

    REGISTER_COMPONENT_WITH_PARENT(MeshCollider, CLASS_TO_HASH(Collider));
    REGISTER_COMPONENT_WITH_PARENT(BoxCollider, CLASS_TO_HASH(Collider));

    REGISTER_COMPONENT_WITH_PARENT(MeshRender, CLASS_TO_HASH(BaseRender));
    REGISTER_COMPONENT_WITH_PARENT(SkinnedMeshRender, CLASS_TO_HASH(BaseRender));
}

#undef REGISTER_COMPONENT_WITH_PARENT

