#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/ecs/gameobjectmanager.hpp"
#include "gr-base/ecs/entitymanager.hpp"

#include "gr-base/core/scenemanager.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/core/file/stream.hpp"

GameObject *GameObject::Create(const std::string &name, const std::string &tag)
{
    return GameObjectManager::Create(name, tag);
}

GameObject *GameObject::Instantiate(const GameObject *go)
{
    auto scene = go->get_scene();
    if (scene == nullptr)
        scene = SceneManager::GetCurrent();
    
    if (scene == nullptr)
        return nullptr;

    auto gameObject = GameObject::Create(go->m_name, go->m_tag);
    if (!gameObject)
    {
        return nullptr;
    }
    
    gameObject->m_static = go->m_static;
    gameObject->m_active = go->m_active;
    gameObject->m_scene = scene;

    gameObject->getEntity()->copy(go->getEntity());

    scene->insert(gameObject);

    if (scene->is_running())
    {
        Scene::Play(gameObject);
    }

    return gameObject;
}

GameObject *GameObject::Find(const std::string &name)
{
    return GameObjectManager::Find(name);
}

void GameObject::Destroy(GameObject *go)
{
    return GameObjectManager::Destroy(go);
}

GameObject::GameObject(const std::string &name, const std::string &tag) : m_name(name), m_tag(tag), m_entity(nullptr), m_active(true), m_static(false), m_transform(nullptr), m_scene(nullptr)
{
    m_entity = EntityManager::Create(this);

    m_transform = addComponent<Transform>();
}

GameObject::~GameObject()
{
    if (m_entity != nullptr)
        EntityManager::Destroy(m_entity);
}

void GameObject::setName(const std::string &name)
{
    m_name = GameObjectManager::Rename(this, name);
}

std::string GameObject::getName() const {
    return m_name;
}

void GameObject::setTag(const std::string &str) {
    m_tag = str;
}

std::string GameObject::getTag() const {
    return m_tag;
}

void GameObject::setTransform(Transform *t) {
    m_transform = t;
}

Transform *GameObject::getTransform() const {
    return m_transform;
}

Entity *GameObject::getEntity() const {
    return m_entity;
}

Scene *GameObject::get_scene() const
{
    return m_scene;
}

void GameObject::setActive(bool b) {
    m_active = b;
}

bool GameObject::getActive() const {
    return m_active;
}

void GameObject::setStatic(bool b) {
    m_static = b;
}

bool GameObject::getStatic() const {
    return m_static;
}

bool GameObject::serialize(Stream *stream) const
{
    if (!stream->write(m_name))
    {
        return false;
    }

    if (!stream->write(m_tag))
    {
        return false;
    }

    if (!stream->write(m_active))
    {
        return false;
    }

    if (!stream->write(m_static))
    {
        return false;
    }

    return true;
}

bool GameObject::deserialize(Stream *stream)
{
    if (!stream->read(m_name))
    {
        return false;
    }

    if (!stream->read(m_tag))
    {
        return false;
    }

    if (!stream->read(m_active))
    {
        return false;
    }

    if (!stream->read(m_static))
    {
        return false;
    }
    return true;
}








