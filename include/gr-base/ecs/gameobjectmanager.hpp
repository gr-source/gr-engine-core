#pragma once

#include "gr-base/core/nameManager.hpp"
#include "gr-base/ecs/gameObject.hpp"

class GameObjectManager
{
public:
    static GameObject *Create(const std::string &name, const std::string &tag);

    static GameObject *Find(const std::string &name);

    static void Destroy(GameObject *gameObject);

    static std::string Rename(GameObject *, const std::string &name);

private:
    std::unordered_map<std::string, std::unique_ptr<GameObject>> m_gameobjectmap;

    std::unordered_map<const GameObject *, const std::string> m_gameobjectnamemap;

    NameManager m_namemanager;

    std::mutex m_mutex;

    static GameObjectManager& GetInstance();

    GameObjectManager();
    ~GameObjectManager();

    GameObject *create(const std::string &name, const std::string &tag);

    GameObject *find(const std::string &name);

    void destroy(GameObject *gameObject);

    std::string rename(GameObject *gameObject, const std::string &name);
};

