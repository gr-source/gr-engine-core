#pragma once

#include <unordered_map>
#include <string>
#include <memory>

class Transform;
class Entity;

class GameObject {
public:
    static GameObject *Create(const std::string &name, const std::string &tag = "unttaged");

    static GameObject *Instantiate(GameObject *go);

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

    void setEnable(bool b);

    bool getEnable() const;

    template <typename T>
    T *addComponent();

    template <typename T>
    T *getComponent() const;
    
    template <typename T>
    bool hasComponent() const;
    
    template <typename T>
    void destroy();
    
private:
    Transform *transform;

    std::string name;

    std::string tag;

    Entity *entity;

    bool active;

    bool enable;

protected:
    static std::unordered_map<std::string, std::unique_ptr<GameObject>> mGoCache;
};

#include "entity.hpp"

template <typename T>
inline T *GameObject::addComponent() {
    return entity->addComponent<T>();
}

template <typename T>
inline T *GameObject::getComponent() const {
    return entity->getComponent<T>();
}

template <typename T>
inline bool GameObject::hasComponent() const {
    return entity->hasComponent<T>();
}

template <typename T>
inline void GameObject::destroy() {
    return entity->destroy<T>();
}
