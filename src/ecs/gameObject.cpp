#include "gr-base/ecs/gameObject.hpp"
#include "gr-base/ecs/manager.hpp"

#include "gr-base/core/nameManager.hpp"

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

    gameObject->transform = gameObject->addComponent<Transform>();
    if (!gameObject->transform) {
        mGoCache.erase(it);
        return nullptr;
    }

    return &(*gameObject);
}

GameObject *GameObject::Instantiate(GameObject *go) {
    auto gameObject = GameObject::Create(go->name, go->tag);
    if (!gameObject) {
        return nullptr;
    }
    
    gameObject->enable = go->enable;
    gameObject->active = go->active;

    gameObject->getEntity()->copy(go->getEntity());

    return gameObject;
}

void GameObject::Rename(GameObject *go, std::string name) {
    auto it = mGoCache.find(go->getName());
    if (it == mGoCache.end()) {
        return;
    }

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

GameObject::GameObject(const std::string &name, const std::string &tag) : name(name), tag(tag), entity(Manager::CreateEntity(this)), active(true), enable(true) {}

GameObject::~GameObject() {
    Manager::DestroyEntity(entity);
}

void GameObject::setName(const std::string &str) {
    name = str;
}

std::string GameObject::getName() const {
    return name;
}

void GameObject::setTag(const std::string &str) {
    tag = str;
}

std::string GameObject::getTag() const {
    return tag;
}

void GameObject::setTransform(Transform *t) {
    transform = t;
}

Transform *GameObject::getTransform() const {
    return transform;
}

Entity *GameObject::getEntity() const {
    return entity;
}

void GameObject::setActive(bool b) {
    active = b;
}

bool GameObject::getActive() const {
    return active;
}

void GameObject::setEnable(bool b) {
    enable = b;
}

bool GameObject::getEnable() const {
    return enable;
}
