#include "gr-base/ecs/component.hpp"
#include "gr-base/ecs/entity.hpp"

GameObject *Component::getGameObject() const {
    return entity->getGameObject();
}


