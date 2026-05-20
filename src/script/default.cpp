#include "gr-base/script/default.hpp"

#include "gr-base/core/logger.hpp"

#include "gr-base/ecs/type.hpp"

#include "gr-base/script/relationship.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/camera.hpp"
#include "gr-base/script/name.hpp"

#define REGISTRY(K) \
        Type::Registry<K>(#K) \

void RegistryDefaultComponent()
{
    logger::debug("[Type::Registry] Carregando script padrões...");

    REGISTRY(Relationship);
    REGISTRY(Transform);
    REGISTRY(Camera);
    REGISTRY(Name);
}

#undef REGISTRY

