#include "gr-base/ecs/behavior.hpp"

#include "gr-base/ecs/entity.hpp"
#include "gr-base/managers/gameobjectmanager.hpp"

GameObject* Behavior::GetGameObject() const
{
    return (GameObject*)GetEntity();
}

Entity* Behavior::GetEntity() const
{
    return (Entity*)GameObjectManager::Get(entityID);
}



