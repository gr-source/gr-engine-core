#pragma once

#include "ecs/gComponent.hpp"

#include <unordered_map>
#include <vector>
#include <memory>

using EntityID = std::size_t;

class Entity {
public:
    
    GameObject *getGameObject() const;
private:
    std::unordered_map<std::string, std::unique_ptr<Component>> m_componentPtr;

    std::vector<Component *> m_components;

    GameObject *m_gameObject;

    EntityID m_entityID;
};




