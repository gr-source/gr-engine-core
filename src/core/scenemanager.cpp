#include "gr-base/core/scenemanager.hpp"

#include "gr-base/core/scene.hpp"

#include <algorithm>

// ***** public ***** //
Scene *SceneManager::Create(const std::string &name)
{
    return GetInstance().create(name);
}

Scene *SceneManager::Find(const std::string &name)
{
    return GetInstance().find(name);
}

const char *SceneManager::GetSceneName(const Scene *scene)
{
    return GetInstance().get_scene_name(scene);
}

std::vector<Scene *> SceneManager::GetScenes()
{
    return GetInstance().get_scenes();
}

void SceneManager::Remove(Scene *scene)
{
    return GetInstance().remove(scene);
}

void SceneManager::SetActive(Scene *scene)
{
    return GetInstance().set_active(scene);
}

Scene *SceneManager::GetCurrent()
{
    return GetInstance().get_current();
}

// ***** private ***** //
SceneManager &SceneManager::GetInstance()
{
    static SceneManager instance;
    return instance;
}

SceneManager::SceneManager()
{
}

SceneManager::~SceneManager()
{
}

Scene *SceneManager::create(const std::string &name)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    auto &&[it, added] = m_scenemap.emplace(name, std::make_unique<Scene>());
    if (added)
    {
        m_sceneList.push_back(it->second.get());

        m_scenenamemap.emplace(it->second.get(), name);

        lock.unlock();

        set_active(it->second.get());

        lock.lock();
    }

    return it->second.get();
}

Scene *SceneManager::find(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_scenemap.find(name);
    if (it == m_scenemap.end())
    {
        return nullptr;
    }
    return it->second.get();
}

const char *SceneManager::get_scene_name(const Scene *scene)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_scenenamemap.find(scene);
    if (it == m_scenenamemap.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

std::vector<Scene *> SceneManager::get_scenes()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sceneList;
}

void SceneManager::remove(Scene *scene)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_scenenamemap.find(scene);
    if (it == m_scenenamemap.end())
    {
        return;
    }

    m_scenenamemap.erase(it);
    m_sceneList.erase(std::remove(m_sceneList.begin(), m_sceneList.end(), scene), m_sceneList.end());
    m_scenemap.erase(it->second);
}

void SceneManager::set_active(Scene *scene)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_current = scene;
}

Scene *SceneManager::get_current()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_current;
}



