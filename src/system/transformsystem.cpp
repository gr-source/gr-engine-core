#include "gr-base/system/transformsystem.hpp"

#include "gr-base/ecs/componentstorage.hpp"
#include "gr-base/script/transform.hpp"

void TransformSystem::Update()
{
    auto sparset = ComponentStorage::GetSparset<Transform>();



    for (ComponentID id = 0;id<sparset->count;id++)
    {
    }
}

