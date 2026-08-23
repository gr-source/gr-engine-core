#include "gr-base/ecs/type_id.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/systemRegistry.hpp"

template <>
uint32_t TypeId<IdCategory::SYSTEM>::Generate(Hash64 hash) noexcept
{
    return SystemRegistry::GetTypeIDByHash(hash);
}

template <>
uint32_t TypeId<IdCategory::COMPONENT>::Generate(Hash64 hash) noexcept
{
    return ComponentRegistry::GetTypeIDByHash(hash);
}

