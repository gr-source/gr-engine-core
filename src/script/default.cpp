#include "gr-base/script/default.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/typeRegistry.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/relationship.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/light.hpp"
#include "gr-base/script/name.hpp"

#define REGISTRY(K, b) \
        TypeRegistry::Registry<K>(#K, b) \

void RegistryDefaultComponent()
{
    logger::debug("[TypeRegistry::Registry] Carregando script padrões...");

    REGISTRY(Relationship, false);

    REGISTRY(MeshRender, false);
    REGISTRY(MaterialRef, false);

    REGISTRY(MeshFilter, false);
    REGISTRY(MeshRef, false);
    
    REGISTRY(CameraData, false);
    REGISTRY(Transform, true);
    REGISTRY(Camera, true);
    REGISTRY(Light, true);
    REGISTRY(Name, false);
}

#undef REGISTRY

