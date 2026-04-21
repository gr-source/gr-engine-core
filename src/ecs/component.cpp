#include "gr-base/ecs/component.hpp"

#include "gr-base/managers/gameobjectmanager.hpp"

GameObject* Component::GetGameObject() const
{
    return GameObjectManager::Get(entityID);
}





