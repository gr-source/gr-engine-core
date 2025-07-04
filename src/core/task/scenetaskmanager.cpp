#include "gr-base/core/task/scenetaskmanager.hpp"

#include "gr-base/core/task/runtime/sceneloadtask.hpp"

#include "gr-base/core/thread/taskmanager.hpp"

std::shared_ptr<TaskInfo> SceneTaskManager::create_task(const std::string &path, std::function<void (Scene *)> func)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_taskMap.find(path);
    if (it != m_taskMap.end())
    {
        auto info = it->second;
        
        auto sceneLoadtask = dynamic_cast<SceneLoadTask *>(info->task.get());
        
        lock.unlock();

        sceneLoadtask->addFunction(func);
        return info;
    }

    auto info = TaskManager::Add<SceneLoadTask>(path);
    auto sceneLoadTask = dynamic_cast<SceneLoadTask *>(info->task.get());

    m_taskMap.emplace(path, info);

    lock.unlock();

    sceneLoadTask->addFunction(func);

    return info;
}

bool SceneTaskManager::remove_task(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_taskMap.find(name);
    if (it == m_taskMap.end())
    {
        return false;
    }
    m_taskMap.erase(it);
    return true;
}


