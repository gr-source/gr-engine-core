#pragma once

#include "gr-base/core/default.hpp"

#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

class SceneManager
{
public:
    static void Initialize();

    static void Shutdown();

    static Scene *Create(const std::string &name);

    static Scene *Find(const std::string &name);

    static const char *GetSceneName(const Scene *scene);

    static std::vector<Scene *> GetScenes();

    static void Remove(Scene *scene);

    static void SetActive(Scene *scene);

    static Scene *GetCurrent();

private:
    static SceneManager* s_SceneManager;

    std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenemap;

    std::unordered_map<const Scene *, const std::string> m_scenenamemap;

    std::vector<Scene *> m_sceneList;

    Scene *m_current;

    SceneManager();
    ~SceneManager();

    Scene *create(const std::string &name);

    Scene *find(const std::string &name);

    const char *get_scene_name(const Scene *scene);

    std::vector<Scene *> get_scenes();

    void remove(Scene *scene);

    void set_active(Scene *scene);

    Scene *get_current();
};



