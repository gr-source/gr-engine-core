#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/default.hpp"

#include "default.hpp"

class GameObject
{
public:
    static GameObject *Create(const std::string &name, const std::string &tag = "unttaged");

    static GameObject *Instantiate(const GameObject *go);

    static GameObject *Find(const std::string &name);

    static void Destroy(GameObject *go);

    GameObject() = delete;
    GameObject(const std::string &name, const std::string &tag = "untagged");
    ~GameObject();

    void setName(const std::string &name);

    std::string getName() const;

    void setTag(const std::string &str);

    std::string getTag() const;

    void setTransform(Transform *t);

    Transform *getTransform() const;

    Entity *getEntity() const;

    Scene *get_scene() const;

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

    bool serialize(Stream *stream) const;

    bool deserialize(Stream *stream);
    
private:
    Transform *m_transform;

    std::string m_name;

    std::string m_tag;

    Entity *m_entity;

    Scene *m_scene;

    bool m_active;

    bool m_static;
};

#include "entity.hpp"

template <typename T>
inline T *GameObject::addComponent()
{
    return m_entity->addComponent<T>();
}

template <typename T>
inline T *GameObject::getComponent() const
{
    return m_entity->getComponent<T>();
}

template <typename T>
inline bool GameObject::hasComponent() const
{
    return m_entity->hasComponent<T>();
}

template <typename T>
inline void GameObject::destroy()
{
    return m_entity->destroy<T>();
}



