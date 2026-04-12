#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include <unordered_map>
#include <functional>
#include <string>

class SceneTaskManager
{
public:
    SceneTaskManager() = default;
    ~SceneTaskManager() = default;

    std::shared_ptr<TaskInfo> create_save_task(const std::string &path, const Scene* scene, std::function<void ()> func);

    std::shared_ptr<TaskInfo> create_task(const std::string &path, std::function<void (Scene *)> func);

    bool remove_task(const std::string &name);

private:
    std::unordered_map<std::string, std::shared_ptr<TaskInfo>> m_taskMap;

    std::mutex m_mutex;
};


