#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

class SceneTaskManager
{
public:
    SceneTaskManager() = default;
    ~SceneTaskManager() = default;

    std::shared_ptr<TaskInfo> create_task(const std::string &path, std::function<void (Scene *)> func);

    bool remove_task(const std::string &name);

private:
    std::unordered_map<std::string, std::shared_ptr<TaskInfo>> m_taskMap;

    std::mutex m_mutex;
};


