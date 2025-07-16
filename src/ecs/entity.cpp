#include "gr-base/ecs/entity.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/transform.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <map>

static std::map<std::string, int> MapScriptPriority
{
    {"Transform",           100},
    {"Camera",              98},
    {"Light",               98},
    {"MeshFilter",          99},
    {"MeshRender",          98},
    {"SkinnedMeshRender",   98},
    {"MeshCollider",        97},
    {"BoxCollider",         97},
    {"Rigidbody",           96}
};

Entity::Entity(GameObject *go, EntityID id) : m_gameObject(go), m_entityID(id)
{
}

Entity::~Entity()
{
    std::vector<std::string> scriptnames;
    for (auto [name, script] : m_componentPtr)
    {
        scriptnames.push_back(name);
    }

    for (auto name : scriptnames)
    {
        destroy(name);
    }
}

Component *Entity::addComponent(const std::string &name, const Component *component)
{
    auto pool = ComponentRegistry::Find(name);
    if (!pool)
    {
        throw std::runtime_error("Fail to add component, " + name + " do not registry.");
    }

    auto instance = pool->allocatein(pool, this, component);
    if (!instance)
    {
        throw std::runtime_error("Fail to add component, " + name + " invalid instance.");
    }
    
    instance->entity = this;

    m_componentPtr[name] = instance;

    m_components.push_back(instance);

    pool->init(instance);

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

template <typename T>
static T _for_each(Transform *transform, std::function<T (Transform *)> func)
{
    for (auto &child : transform->getChildren())
    {
        auto ret = func(child);
        if (ret != nullptr)
        {
            return ret;
        }

        ret = _for_each(child, func);
        if (ret != nullptr)
            return ret;
    }

    return nullptr;
}

template <typename T, typename ...Args>
static T for_each(Transform *transform, Args &&...args)
{
    std::function<T (Transform *)> func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);

    return _for_each(transform, func);
}

Component *find(const std::string &name, Transform *transform)
{
    auto entity = transform->entity;
    if (entity->hasComponent(name))
    {
        return entity->getComponent(name);
    }
    return nullptr;
}

Component *Entity::getComponentInChildren(const std::string &name) const
{
    auto gameObject = getGameObject();

    auto transform = gameObject->getTransform();
    return for_each<Component *>(transform, &find, name);
}

std::unordered_map<std::string, Component *> Entity::getComponentMap() const {
    return m_componentPtr;
}

GameObject *Entity::getGameObject() const
{
    return m_gameObject;
}

EntityID Entity::getEntityID() const
{
    return m_entityID;
}

void Entity::setEnable(bool value) {
    for (auto [name, component] : m_componentPtr)
    {
        auto pool = ComponentRegistry::Find(name);
        if (pool == nullptr)
        {
            continue;
        }

        value ? pool->enable(component) : pool->disable(component);
    }
}


void Entity::play()
{
    std::vector<std::tuple<std::string, Component *>> temp;
    for (auto &&[name, component] : m_componentPtr)
    {
        temp.emplace_back(name, &(*component));
    }

    std::sort(temp.begin(), temp.end(), [](auto &a, auto &b)
        {
           return MapScriptPriority[std::get<std::string>(a)] > MapScriptPriority[std::get<std::string>(b)];
        }
    );

    for (auto [name, component] : temp)
    {
        auto pool = ComponentRegistry::Find(name);
        if (!pool)
        {
            continue;
        }

        try
        {
            pool->start(component);
        } catch (const std::exception &e)
        {
            throw std::runtime_error("Fail to initialize " + name + ", " + e.what());
        }
    }
}

void Entity::stop()
{
    std::vector<std::tuple<std::string, Component *>> temp;
    for (auto &&[name, component] : m_componentPtr) {
        temp.emplace_back(name, &(*component));
    }

    std::sort(temp.begin(), temp.end(), [](auto &a, auto &b)
        {
           return MapScriptPriority[std::get<std::string>(a)] > MapScriptPriority[std::get<std::string>(b)];
        }
    );

    for (auto [name, component] : temp)
    {
        auto pool = ComponentRegistry::Find(name);
        if (pool)
        {
            pool->destroy(component);
        }
    }
}

void Entity::copy(const Entity *entity)
{
    for (auto &&[name, component] : entity->m_componentPtr)
    {
        if (hasComponent(name))
        {
            continue;
        }

        addComponent(name, component);
    }

    auto go = entity->getGameObject();

    auto targetTransform = go->getTransform();
    
    auto localTransform = m_gameObject->getTransform();
    localTransform->setLocalScale(targetTransform->getLocalScale());
    localTransform->setLocalRotation(targetTransform->getLocalRotation());
    localTransform->setLocalPosition(targetTransform->getLocalPosition());

    if (targetTransform->hasParent())
    {
        localTransform->setParent(targetTransform->getParent());
    }

    for (const auto& child : targetTransform->getChildren())
    {
        auto childGameObject = child->getGameObject();

        auto clonedObject = GameObject::Create(childGameObject->getName(), childGameObject->getTag());
        if (!clonedObject)
        {
            Console::LogError("[ Entity ] Failed to create child object %s->%s.", go->getName().c_str(), childGameObject->getName().c_str());
            continue;
        }

        clonedObject->getEntity()->copy(childGameObject->getEntity());

        auto clonedTransform = clonedObject->getTransform();
        clonedTransform->setParent(localTransform);
    }
}

void Entity::destroy(const std::string &name)
{
    auto it = m_componentPtr.find(name);
    if (it == m_componentPtr.end())
    {
        return;
    }

    auto storage = ComponentRegistry::Find(name);
    if (storage != nullptr)
    {
        auto scene = getGameObject()->get_scene();
        if (scene != nullptr && scene->is_running())
        {
            storage->destroy(it->second);
        }

        storage->release(it->second);

        it->second->bCanUseUpdate = false;
        it->second->entity = nullptr;

        storage->deallocatein(storage, it->second);
    }

    m_components.erase(std::remove(m_components.begin(), m_components.end(), it->second), m_components.end());

    m_componentPtr.erase(it);
}

void Entity::free(const std::string &name)
{
    auto it = m_componentPtr.find(name);
    if (it == m_componentPtr.end()) {
        return;
    }

    m_components.erase(std::remove(m_components.begin(), m_components.end(), it->second), m_components.end());

    m_componentPtr.erase(it);
}


