#include "gr-base/ecs/pool.hpp"

#include "gr-base/ecs/component.hpp"

Pool::Pool(const std::size_t &size, const std::string &name) : init(nullptr), start(nullptr), update(nullptr), enable(nullptr), disable(nullptr), serialize(nullptr), deserialize(nullptr), destroy(nullptr), m_size(size), m_name(name) {}

std::vector<Field> Pool::getFields() const {
    return m_fieldMap;
}

std::size_t Pool::getSize() const {
    return m_size;
}

std::string Pool::getName() const {
    return m_name;
}

Component *Pool::CreatComponent(std::size_t size, Entity *entity) {
    auto component = reinterpret_cast<Component *>(std::malloc(size));
    if (!component) {
        return nullptr;
    }
    component->entity = entity;
    return component;
}
