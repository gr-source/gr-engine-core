#include "ecs/entity.hpp"
#include "ecs/componentRegistry.hpp"

#include <list>

Entity::Entity(GameObject *go, EntityID id) : m_gameObject(go), m_entityID(id) { }

Entity::~Entity() {
    for (auto c : m_components) {
        std::free(c);
    }
}

Component *Entity::addComponent(const std::string &name, const Component *component) {
    auto pool = ComponentRegistry::Find(name);
    if (!pool) {
        throw std::runtime_error("Fail to add component, " + name + " do not registry.");
    }
    auto instance = pool->CreateInstance(component, this);
    if (!instance) {
        throw std::runtime_error("Fail to add component, " + name + " invalid instance.");
    }
    instance->entity = this;

    m_componentPtr[name] = instance;

    m_components.push_back(instance);

    return instance;
}

Component *Entity::getComponent(const std::string &name) const {
    auto it = m_componentPtr.find(name);
    if (it == m_componentPtr.end()) {
        return nullptr;
    }
    return it->second;
}

bool Entity::hasComponent(const std::string &name) const {
    return m_componentPtr.find(name) != m_componentPtr.end();
}

void Entity::destroy(const std::string &name) {
    auto it = m_componentPtr.find(name);
    if (it == m_componentPtr.end()) {
        return;
    }
    m_components.erase(std::remove(m_components.begin(), m_components.end(), it->second), m_components.end());

    m_componentPtr.erase(it);
}

std::unordered_map<std::string, Component *> Entity::getComponentMap() const {
    return m_componentPtr;
}

GameObject *Entity::getGameObject() const {
    return m_gameObject;
}

EntityID Entity::getEntityID() const {
    return m_entityID;
}

void Entity::setEnable(bool value) {
    for (auto [name, component] : m_componentPtr) {
        auto pool = ComponentRegistry::Find(name);
        if (pool) {
            value ? pool->enable(component) : pool->disable(component);
        }
    }
}

void Entity::init() {
    std::list<std::tuple<std::string, Component *>> temp;
    for (auto &&[name, component] : m_componentPtr) {
        temp.emplace_front(name, &(*component));
    }

    for (auto [name, component] : temp) {
        auto pool = ComponentRegistry::Find(name);
        if (!pool) {
            continue;
        }

        try {
            pool->init(component);
        } catch (const std::exception &e) {
            throw std::runtime_error("Fail to initialize " + name + ", " + e.what());
        }
    }
}

void Entity::update() {
    for (auto &&[name, component] : m_componentPtr) {
        auto pool = ComponentRegistry::Find(name);
        if (!pool) {
            continue;
        }
        pool->update(&(*component));
    }
}

void Entity::destroy() {
    std::list<std::tuple<std::string, Component *>> temp;
    for (auto &&[name, component] : m_componentPtr) {
        temp.emplace_front(name, &(*component));
    }

    for (auto [name, component] : temp) {
        auto pool = ComponentRegistry::Find(name);
        if (pool) {
            pool->destroy(component);
        }
    }
}
