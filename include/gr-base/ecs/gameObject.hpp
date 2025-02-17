#pragma once

#include <unordered_map>
#include <string>
#include <memory>

class Transform;
class Entity;

class GameObject {
public:
    static GameObject *Create(const std::string &name, const std::string &tag = "unttaged");

    static GameObject *Instantiate(const GameObject *go);

    static void Rename(GameObject *go, std::string name);

    static GameObject *Find(const std::string &name);

    static void Destroy(GameObject *go);

    static void Clear();

    static std::size_t GetGameObjectCount();

    GameObject() = delete;
    GameObject(const std::string &name, const std::string &tag = "untagged");
    ~GameObject();

    void setName(const std::string &str);

    std::string getName() const;

    void setTag(const std::string &str);

    std::string getTag() const;

    void setTransform(Transform *t);

    Transform *getTransform() const;

    Entity *getEntity() const;

    void setActive(bool b);

    bool getActive() const;

    void setStatic(bool b);

    bool getStatic() const;

    template <typename T>
    T *addComponent();

    template <typename T>
    T *getComponent() const;
    
    template <typename T>
    bool hasComponent() const;
    
    template <typename T>
    void destroy();
    
private:
    Transform *m_transform;

    std::string m_name;

    std::string m_tag;

    Entity *m_entity;

    bool m_active;

    bool m_static;

protected:
    static std::unordered_map<std::string, std::unique_ptr<GameObject>> mGoCache;
};

#include "entity.hpp"

template <typename T>
inline T *GameObject::addComponent() {
    return m_entity->addComponent<T>();
}

template <typename T>
inline T *GameObject::getComponent() const {
    return m_entity->getComponent<T>();
}

template <typename T>
inline bool GameObject::hasComponent() const {
    return m_entity->hasComponent<T>();
}

template <typename T>
inline void GameObject::destroy() {
    return m_entity->destroy<T>();
}
