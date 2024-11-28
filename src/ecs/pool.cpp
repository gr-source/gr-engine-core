#include "ecs/pool.hpp"

#include "ecs/component.hpp"

std::vector<Field> Pool::getFields() const {
    return m_fieldMap;
}

Component *Pool::CreatComponent(std::size_t size, Entity *entity) {
    auto component = reinterpret_cast<Component *>(std::malloc(size));
    if (!component) {
        return nullptr;
    }
    component->entity = entity;
    return component;
}

Pool::Pool() : init(nullptr), start(nullptr), update(nullptr), enable(nullptr), disable(nullptr), serialize(nullptr), deserialize(nullptr), destroy(nullptr) {}

