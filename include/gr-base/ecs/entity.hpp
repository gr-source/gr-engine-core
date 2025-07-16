#pragma once

#include "gr-base/ecs/component.hpp"

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

    Component *getComponentInChildren(const std::string &name) const;

    template <typename T>
    T *getComponentInChildren() const;

    std::unordered_map<std::string, Component *> getComponentMap() const;

    GameObject *getGameObject() const;

    EntityID getEntityID() const;

    void setEnable(bool value);

    void play();

    void stop();

    void copy(const Entity *entity);

    void serialize(Stream *stream) const;

    void deserialize(Stream *stream);

    void destroy(const std::string &name);

    void free(const std::string &name);
    
private:
    std::unordered_map<std::string, Component *> m_componentPtr;

    std::vector<Component *> m_components;

    GameObject *m_gameObject;

    EntityID m_entityID;
};

#include "gr-base/core/string.hpp"

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
inline T *Entity::getComponentInChildren() const
{
    static_assert(sizeof(T) != 0, "Component don't defined.");

    return static_cast<T *>(getComponentInChildren(String::RemoveNumber(typeid(T).name())));
}

template <typename T>
inline void Entity::destroy() {
    static_assert(sizeof(T) != 0, "Component don't defined.");

    return destroy(String::RemoveNumber(typeid(T).name()));
}




