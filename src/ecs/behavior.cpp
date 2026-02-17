#include "gr-base/ecs/behavior.hpp"

#include "gr-base/managers/gameobjectmanager.hpp"

GameObject* Behavior::GetGameObject() const
{
    return GameObjectManager::Get(entityID);
}





