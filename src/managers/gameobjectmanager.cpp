#include "gr-base/managers/gameobjectmanager.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/default.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/script/transform.hpp"

#include "gr-base/managers/sceneManager.hpp"

#include <string.h>

typedef struct GameObjectWrapper
{
    uint32_t next;

    GameObject gameObject;
} GameObjectWrapper;

GameObjectManager* GameObjectManager::s_GameObjectManager = nullptr;

void GameObjectManager::Initialize()
{
    if (s_GameObjectManager == nullptr)
        s_GameObjectManager = new GameObjectManager();
}

void GameObjectManager::Shutdown()
{
    if (s_GameObjectManager != nullptr)
        delete s_GameObjectManager;
    s_GameObjectManager = nullptr;
}

GameObjectID GameObjectManager::Instantiate(const GameObject *other)
{
    return s_GameObjectManager->instantiate(other->getCacheID());
}

GameObjectID GameObjectManager::Create(const std::string &name)
{
    return s_GameObjectManager->create(name);
}

GameObject* GameObjectManager::Get(GameObjectID id)
{
    return s_GameObjectManager->get(id);
}

void GameObjectManager::DestroyImediate(GameObjectID id)
{
    s_GameObjectManager->destroyImediate(id);
}

void GameObjectManager::Destroy(GameObjectID id)
{
    s_GameObjectManager->destroy(id);
}

size_t GameObjectManager::GetCount()
{
    return s_GameObjectManager->m_count;
}

// public
GameObjectManager::GameObjectManager() : m_gameObjectPool(nullptr), m_freeIDs(n32nill), m_count(0)
{
    m_gameObjectPool = array_create(1024, sizeof(GameObjectWrapper));

    addFreeID(0);
}

GameObjectManager::~GameObjectManager()
{
    array_free(m_gameObjectPool);
}

// private
GameObjectID GameObjectManager::instantiate(GameObjectID other)
{
    auto handle = create(GameObject::Get(other)->getName());
    if (handle == GID_INVALID)
        return GID_INVALID;

    auto gameObject = get(handle);
    gameObject->Copy(other);

    auto scene = SceneManager::GetCurrent();
    if (scene != nullptr)
        scene->insert(handle);

    return handle;
}

GameObjectID GameObjectManager::create(const std::string &name)
{
    if (m_freeIDs == n32nill)
        requestResize();

    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_gameObjectPool->vector;

    uint32_t id = m_freeIDs;
    m_freeIDs = wrapper[id].next;

    GameObjectID cacheID = CreateGameObjectID(id, 0);

    GameObject* gameObject = &wrapper[id].gameObject;

    new (gameObject) GameObject(cacheID);
    gameObject->setName(name);

    m_count++;

    return cacheID;
}

GameObject* GameObjectManager::get(GameObjectID id)
{
    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_gameObjectPool->vector;

    return &wrapper[GetIDIndex(id)].gameObject;
}

void GameObjectManager::destroyImediate(GameObjectID id)
{
    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_gameObjectPool->vector;

    uint32_t index = GetIDIndex(id);

    auto& gameObject = wrapper[index].gameObject;

    {
        auto transform = gameObject.getTransform();
        for (auto child : transform->getChildren())
            destroyImediate(child->GetGameObject()->getCacheID());
    }

    wrapper[index].next = m_freeIDs;
    m_freeIDs = index;

    gameObject.~GameObject();

    memset((void *)&gameObject, 0, sizeof(GameObject));

    m_count--;
}

void GameObjectManager::destroy(GameObjectID id)
{}

void GameObjectManager::requestResize()
{
    size_t lastsize = m_gameObjectPool->length;
    size_t newsize = lastsize + 1024;

    // m_gameObjectPool = array_resize(m_gameObjectPool, newsize, sizeof(GameObjectWrapper));

    auto newPool = array_create(newsize, sizeof(GameObjectWrapper));

    GameObjectWrapper* oldWrapperPool = (GameObjectWrapper*)m_gameObjectPool->vector;
    GameObjectWrapper* newWrapperPool = (GameObjectWrapper*)newPool->vector;
    for (size_t i=0;i<lastsize;i++)
    {
        GameObjectWrapper& oldwrapper = oldWrapperPool[i];
        GameObjectWrapper& newwrapper = newWrapperPool[i];

        newwrapper.next = oldwrapper.next;

        new (&newwrapper.gameObject) GameObject(oldwrapper.gameObject);
    }

    array_free(m_gameObjectPool);
    m_gameObjectPool = newPool;

    addFreeID(lastsize);
}

void GameObjectManager::addFreeID(uint32_t start)
{
    GameObjectWrapper* wrapper = (GameObjectWrapper*)m_gameObjectPool->vector;

    size_t lastID = m_gameObjectPool->length - 1;
    for (size_t i=start;i<lastID;i++)
        wrapper[i].next = i + 1;

    wrapper[lastID].next = m_freeIDs;
    m_freeIDs = start;
}


