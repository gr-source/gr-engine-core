#include "ecs/component.hpp"
#include "ecs/entity.hpp"

GameObject *Component::getGameObject() const {
    return entity->getGameObject();
}


