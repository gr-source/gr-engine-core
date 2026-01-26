#pragma once

#include "gr-base/core/default.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

#include <string>
#include <atomic>
#include <mutex>

#define n32nill (uint32_t)-1

class GameObjectManager
{
public:
    // static void Initialize();

    // static void Shutdown();

    static void SetInstance(GameObjectManager* gameObjectManager);

    static GameObjectID Instantiate(const GameObject* other);

    static GameObjectID Create(const std::string& name);

    static GameObject* Get(GameObjectID id);

    static void DestroyImediate(GameObjectID id);

    static void Destroy(GameObjectID id);

    static size_t GetCount();

    GameObjectManager();
    ~GameObjectManager();

protected:
    static GameObjectManager* s_GameObjectManager;

    std::mutex m_mutex;

    array* m_gameObjectPool;
    array* m_slot;

    uint32_t m_freeIDs;

    std::atomic<size_t> m_count;

    GameObjectID instantiate(GameObjectID other);

    GameObjectID create(const std::string& name);

    GameObject* get(GameObjectID id);

    void destroyImediate(GameObjectID id);

    void destroy(GameObjectID id);

    void requestResize();

    void addFreeID(uint32_t start);
};




