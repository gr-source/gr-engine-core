#include "gr-base/ecs/gameobjectmanager.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/transform.hpp"

// public
GameObject *GameObjectManager::Create(const std::string &name, const std::string &tag)
{
    return GetInstance().create(name, tag);
}

GameObject *GameObjectManager::Find(const std::string &name)
{
    return GetInstance().find(name);
}

void GameObjectManager::Destroy(GameObject *gameObject)
{
    return GetInstance().mark(gameObject);
}

std::string GameObjectManager::Rename(GameObject * gameObject, const std::string &name)
{
    return GetInstance().rename(gameObject, name);
}

std::size_t GameObjectManager::GetCount()
{
    return GetInstance().get_count();
}

// private
GameObjectManager &GameObjectManager::GetInstance()
{
    static GameObjectManager instance;
    return instance;
}

GameObjectManager::GameObjectManager()
{}

GameObjectManager::~GameObjectManager()
{}

GameObject *GameObjectManager::create(const std::string &name, const std::string &tag)
{
    auto newname = m_namemanager.GenerateNameID(name);
    while (find(newname) != nullptr)
    {
        newname = m_namemanager.GenerateNameID(newname);
    }

    GameObject *gameObject = nullptr;

    {
        auto ptr = std::make_unique<GameObject>(newname, tag);

        std::lock_guard<std::mutex> lock(m_mutex);
        auto &&[it, added] = m_gameobjectmap.emplace(newname, std::move(ptr));
        if (added)
        {
            m_gameobjectnamemap.emplace(it->second.get(), newname);
        }
        gameObject = it->second.get();
    }

    return gameObject;
}

GameObject *GameObjectManager::find(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_gameobjectmap.find(name);
    if (it == m_gameobjectmap.end())
    {
        return nullptr;
    }

    return it->second.get();
}

void GameObjectManager::destroy(GameObject *gameObject)
{
    m_namemanager.RemoveNameID(gameObject->getName());

    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_gameobjectnamemap.find(gameObject);
    if (it == m_gameobjectnamemap.end())
    {
        return;
    }
    lock.unlock();

    auto scene = gameObject->get_scene();
    if (scene != nullptr)
        scene->remove(gameObject);

    auto transform = gameObject->getTransform();
    // remove o objeto pai do objeto altual e se o pai me ter como filho me remove do objeto pai.
    transform->setParent(nullptr);

    auto children = transform->getChildren();

    for (auto child : children) {
        // como o objeto pai ja foi removido diz, aos filhos que não tem mais parent.
        child->setParent(nullptr, false);

        auto gameObject = child->getGameObject();

        GameObject::Destroy(gameObject);
    }

    lock.lock();

    m_gameobjectmap.erase(it->second);

    m_gameobjectnamemap.erase(it);
}

void GameObjectManager::mark(GameObject *gameObject)
{
    MainThreadDispatcher::Add(&GameObjectManager::destroy, this, gameObject);
}

std::string GameObjectManager::rename(GameObject *gameObject, const std::string &name)
{
    auto lastname = gameObject->getName();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_gameobjectmap.find(lastname);
        if (it == m_gameobjectmap.end())
        {
            return lastname;
        }
    }

    m_namemanager.RemoveNameID(lastname);

    auto newname = name;
    while (find(newname) != nullptr)
    {
        newname = m_namemanager.GenerateNameID(newname);
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto pointer = std::move(m_gameobjectmap[lastname]);
    m_gameobjectmap.erase(lastname);
    m_gameobjectnamemap.erase(gameObject);

    auto &&[iterator, added] = m_gameobjectmap.emplace(newname, std::move(pointer));
    if (added)
    {
        m_gameobjectnamemap.emplace(iterator->second.get(), newname);
    }

    return newname;
}

std::size_t GameObjectManager::get_count()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_gameobjectnamemap.size();
}


