#include "ecs/component.hpp"
#include "ecs/gEntity.hpp"

GameObject *Component::getGameObject() const {
    return entity->getGameObject();
}


