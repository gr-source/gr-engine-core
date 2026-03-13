#include "gr-base/managers/sceneManager.hpp"

#include "gr-base/core/scene.hpp"

#include <algorithm>

SceneManager* SceneManager::s_SceneManager = nullptr;

// ***** public ***** //
void SceneManager::SetInstance(SceneManager *sceneManager)
{
    s_SceneManager = sceneManager;
}

Scene *SceneManager::Create(const std::string &name)
{
    return s_SceneManager->create(name);
}

Scene *SceneManager::Find(const std::string &name)
{
    return s_SceneManager->find(name);
}

const char *SceneManager::GetSceneName(const Scene *scene)
{
    return s_SceneManager->get_scene_name(scene);
}

std::vector<Scene *> SceneManager::GetScenes()
{
    return s_SceneManager->get_scenes();
}

void SceneManager::Remove(Scene *scene)
{
    return s_SceneManager->remove(scene);
}

void SceneManager::SetActive(Scene *scene)
{
    return s_SceneManager->set_active(scene);
}

Scene *SceneManager::GetCurrent()
{
    return s_SceneManager->get_current();
}

// ***** private ***** //
SceneManager::SceneManager() : m_current(nullptr)
{}

SceneManager::~SceneManager()
{}

Scene *SceneManager::create(const std::string &name)
{
    auto &&[it, added] = m_scenemap.emplace(name, std::make_unique<Scene>());
    if (added)
    {
        m_sceneList.push_back(it->second.get());

        m_scenenamemap.emplace(it->second.get(), name);

        set_active(it->second.get());
    }

    return it->second.get();
}

Scene *SceneManager::find(const std::string &name)
{
    auto it = m_scenemap.find(name);
    if (it == m_scenemap.end())
    {
        return nullptr;
    }
    return it->second.get();
}

const char *SceneManager::get_scene_name(const Scene *scene)
{
    auto it = m_scenenamemap.find(scene);
    if (it == m_scenenamemap.end())
    {
        return nullptr;
    }
    return it->second.c_str();
}

std::vector<Scene *> SceneManager::get_scenes()
{
    return m_sceneList;
}

void SceneManager::remove(Scene *scene)
{
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
    m_current = scene;
}

Scene *SceneManager::get_current()
{
    return m_current;
}



