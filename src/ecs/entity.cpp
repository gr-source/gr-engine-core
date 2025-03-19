#include "gr-base/ecs/entity.hpp"

#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/componentRegistry.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/transform.hpp"

#include <algorithm>
#include <list>
#include <string>

Entity::Entity(GameObject *go, EntityID id) : m_gameObject(go), m_entityID(id)
{
}

Entity::~Entity() {
    std::vector<std::string> scriptnames;
    for (auto [name, script] : m_componentPtr) {
        scriptnames.push_back(name);
    }

    for (auto name : scriptnames)
    {
        destroy(name);
    }
}

Component *Entity::addComponent(const std::string &name, const Component *component) {
    auto pool = ComponentRegistry::Find(name);
    if (!pool) {
        throw std::runtime_error("Fail to add component, " + name + " do not registry.");
    }

    auto instance = pool->allocatein(pool, this, component);
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

    auto pool = ComponentRegistry::Find(name);
    if (pool != nullptr) {
        pool->destroy(it->second);

        it->second->bCanUseUpdate = false;
        it->second->entity = nullptr;

        pool->deallocatein(pool, it->second);
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

static std::map<std::string, int> MapScriptPriority
{
    {"Transform",       100},
    {"MeshRender",      98},
    {"Camera",          98},
    {"Light",           98},
    {"MeshFilter",      99},
    {"MeshRender",      98},
    {"MeshCollider",    97},
    {"BoxCollider",     97},
    {"Rigidbody",       96}
};

void Entity::init() {
    std::vector<std::tuple<std::string, Component *>> temp;
    for (auto &&[name, component] : m_componentPtr) {
        temp.emplace_back(name, &(*component));
    }

    std::sort(temp.begin(), temp.end(), [](auto &a, auto &b)
        {
           return MapScriptPriority[std::get<std::string>(a)] > MapScriptPriority[std::get<std::string>(b)];
        }
    );

    for (auto [name, component] : temp) {
        auto pool = ComponentRegistry::Find(name);
        if (!pool) {
            continue;
        }

        std::cout << getGameObject()->getName() << ": " << name << std::endl;

        try {
            pool->init(component);
        } catch (const std::exception &e) {
            throw std::runtime_error("Fail to initialize " + name + ", " + e.what());
        }
    }
}

void Entity::destroy() {
    // /*
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
    // */
}

void Entity::copy(const Entity *entity)
{
    for (auto &&[name, component] : entity->m_componentPtr) {
        if (hasComponent(name)) {
            continue;
        }

        addComponent(name, component);
    }

    auto go = entity->getGameObject();

    auto targetTransform = go->getTransform();
    auto localTransform = m_gameObject->getTransform();

    if (targetTransform)
    {
        localTransform->setLocalScale(targetTransform->getLocalScale());
        localTransform->setLocalRotation(targetTransform->getLocalRotation());
        localTransform->setLocalPosition(targetTransform->getLocalPosition());

        if (!targetTransform->hasParent())
        {
            auto scene = Scene::GetCurrentScene();
            if (scene != nullptr)
            {
                scene->insert(getGameObject());
            }
        }

        for (const auto& child : targetTransform->getChildren())
        {
            auto childGameObject = child->getGameObject();
            auto clonedObject = GameObject::Instantiate(childGameObject);
            if (!clonedObject)
            {
                Console::LogError("[ Entity ] Failed to create child object %s->%s.", go->getName().c_str(), childGameObject->getName().c_str());
                continue;
            }

            auto clonedTransform = clonedObject->getTransform();
            if (clonedTransform) {
                clonedTransform->setParent(localTransform);
            }
        }
    } else {
        Console::LogError("[ Entity ] Target game object does not have a transform.");
    }
}




