#include "gr-base/ecs/component.hpp"

#include "gr-base/ecs/entity.hpp"

Component::Component() : bCanUseUpdate(false)
{
}

GameObject *Component::getGameObject() const {
    return entity->getGameObject();
}


