#include "ecs/gComponent.hpp"
#include "ecs/gEntity.hpp"

GameObject *Component::getGameObject() const {
    return entity->getGameObject();
}


