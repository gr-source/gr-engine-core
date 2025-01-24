#pragma once

#include "ecs/component.hpp"

#include <unordered_map>
#include <vector>
#include <memory>

using EntityID = std::size_t;

class Entity {
public:
    Entity() = delete;

    Entity(GameObject *go, EntityID id);
    ~Entity();

    template <typename T>
    T *addComponent();

    template <typename T>
    T *getComponent() const;

    template <typename T>
    bool hasComponent() const;

    template <typename T>
    void destroy();

    Component *addComponent(const std::string &name, const Component *component = nullptr);

    Component *getComponent(const std::string &name) const;

    bool hasComponent(const std::string &name) const;

    void destroy(const std::string &name);

    std::unordered_map<std::string, Component *> getComponentMap() const;

    GameObject *getGameObject() const;

    EntityID getEntityID() const;

    void setEnable(bool value);

    void init();

    void update();

    void destroy();

    void copy(const Entity *entity);
    
private:
    std::unordered_map<std::string, Component *> m_componentPtr;

    std::vector<Component *> m_components;

    GameObject *m_gameObject;

    EntityID m_entityID;
};

#include "core/string.hpp"

template <typename T>
inline T *Entity::addComponent() {
    static_assert(sizeof(T) != 0, "Component don't defined.");

    return static_cast<T *>(addComponent(String::RemoveNumber(typeid(T).name())));
}

template <typename T>
inline T *Entity::getComponent() const {
    static_assert(sizeof(T) != 0, "Component don't defined.");

    return static_cast<T *>(getComponent(String::RemoveNumber(typeid(T).name())));
}

template <typename T>
inline bool Entity::hasComponent() const {
    static_assert(sizeof(T) != 0, "Component don't defined.");

    return hasComponent(String::RemoveNumber(typeid(T).name()));
}

template <typename T>
inline void Entity::destroy() {
    static_assert(sizeof(T) != 0, "Component don't defined.");

    return destroy(String::RemoveNumber(typeid(T).name()));
}
