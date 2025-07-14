#pragma once

#include "gr-base/core/nameManager.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include <unordered_set>
#include <atomic>

class GameObjectManager
{
public:
    static GameObject *Create(const std::string &name, const std::string &tag);

    static GameObject *Find(const std::string &name);

    static void Destroy(GameObject *gameObject);

    static std::string Rename(GameObject *, const std::string &name);

    static std::size_t GetCount();

    static void Initialize();

    static void Shutdown();

private:
    static GameObjectManager *m_instance;

    std::unordered_map<std::string, std::unique_ptr<GameObject>> m_gameobjectmap;

    std::unordered_map<const GameObject *, const std::string> m_gameobjectnamemap;

    NameManager m_namemanager;

    std::mutex m_mutex;

    GameObjectManager();
    ~GameObjectManager();

    GameObject *create(const std::string &name, const std::string &tag);

    GameObject *find(const std::string &name);

    void destroy(GameObject *gameObject);

    void mark(GameObject *gameObject);

    std::string rename(GameObject *gameObject, const std::string &name);

    std::size_t get_count();
};

