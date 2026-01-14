#pragma once

#include "gr-base/core/default.hpp"

extern "C"
{
#include <gr-ecs/array.h>
}

#include <string>
#include <atomic>

#define n32nill (uint32_t)-1

class GameObjectManager
{
public:
    static void Initialize();

    static void Shutdown();

    static GameObject* Instantiate(const GameObject* other);

    static GameObjectID Create(const std::string& name);

    static GameObject* Get(GameObjectID id);

    static void DestroyImediate(GameObjectID id);

    static void Destroy(GameObjectID id);

    static size_t GetCount();

protected:
    static GameObjectManager* s_GameObjectManager;

    GameObjectManager();
    ~GameObjectManager();

    array* m_gameObjectPool;

    uint32_t m_freeIDs;

    std::atomic<size_t> m_count;

    GameObject* instantiate(const GameObject* other);

    GameObjectID create(const std::string& name);

    GameObject* get(GameObjectID id);

    void destroyImediate(GameObjectID id);

    void destroy(GameObjectID id);

    void requestResize();

    void addFreeID(uint32_t start);
};




