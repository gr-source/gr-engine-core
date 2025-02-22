#include "gr-base/ecs/gameObject.hpp"
#include "gr-base/ecs/manager.hpp"

#include "gr-base/core/nameManager.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/transform.hpp"

std::unordered_map<std::string, std::unique_ptr<GameObject>> GameObject::mGoCache;

GameObject *GameObject::Create(const std::string &name, const std::string &tag) {
    auto newName = NameManager::GenerateNameID(name);

    while (Find(newName)) {
        newName = NameManager::GenerateNameID(newName);
    }

    auto &&[it, added] = mGoCache.emplace(newName, std::make_unique<GameObject>(newName, tag));
    if (!added) {
        throw std::runtime_error("GameObject has been replaced: " + newName);
    }

    auto &gameObject = it->second;

    return &(*gameObject);
}

GameObject *GameObject::Instantiate(const GameObject *go) {
    auto gameObject = GameObject::Create(go->m_name, go->m_tag);
    if (!gameObject) {
        return nullptr;
    }
    
    gameObject->m_static = go->m_static;
    gameObject->m_active = go->m_active;

    gameObject->getEntity()->copy(go->getEntity());

    Scene::Init(gameObject);

    return gameObject;
}

void GameObject::Rename(GameObject *go, std::string name) {
    auto it = mGoCache.find(go->getName());
    if (it == mGoCache.end()) {
        return;
    }

    NameManager::RemoveNameID(name);

    name = NameManager::GenerateNameID(name);

    auto ptr = std::move(it->second);

    mGoCache.erase(it);

    mGoCache.emplace(name, std::move(ptr));
    
    go->setName(name);
}

GameObject *GameObject::Find(const std::string &name) {
    auto it = mGoCache.find(name);
    return it != mGoCache.end() ? &(*it->second) : nullptr;
}

#include "gr-base/core/scene.hpp"
void GameObject::Destroy(GameObject *go) {
    Scene::Remove(go);

    auto sname = go->getName();

    auto it = mGoCache.find(sname);
    if (it == mGoCache.end()) {
        return;
    }

    NameManager::RemoveNameID(sname);

    auto transform = go->getTransform();
    // remove o objeto pai do objeto altual e se o pai me ter como filho me remove do objeto pai.
    transform->setParent(nullptr);

    auto children = transform->getChildren();

    for (auto child : children) {
        // como o objeto pai ja foi removido diz, aos filhos que não tem mais parent.
        child->setParent(nullptr, false);

        auto gameObject = child->getGameObject();

        GameObject::Destroy(gameObject);
    }

    mGoCache.erase(it);
}

void GameObject::Clear() {
    mGoCache.clear();

    NameManager::Shutdown();
    
}

std::size_t GameObject::GetGameObjectCount() {
    return mGoCache.size();
}

GameObject::GameObject(const std::string &name, const std::string &tag) : m_name(name), m_tag(tag), m_entity(Manager::CreateEntity(this)), m_active(true), m_static(true) {
    m_transform = addComponent<Transform>();
}

GameObject::~GameObject() {
    Manager::DestroyEntity(m_entity);
}

void GameObject::setName(const std::string &str) {
    m_name = str;
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
