#pragma once

#include "gr-base/core/thread/asynctask.hpp"

class TaskManager
{
public:
    static void Initialize();
    
    static void Shutdown();

    template <typename T, typename ...Args>
    static std::shared_ptr<TaskInfo> Add(Args &&...args);

private:
    static TaskManager* s_TaskManager;

    std::shared_ptr<TaskInfo> add(std::shared_ptr<ITask> task);

    void update(std::shared_ptr<TaskInfo> info);

    void finish(std::shared_ptr<TaskInfo> info);
};

// static
template <typename T, typename ...Args>
inline std::shared_ptr<TaskInfo> TaskManager::Add(Args &&...args)
{
    static_assert(std::is_base_of<ITask, T>(), "This class does not inherit from ITask!");

    return s_TaskManager->add(std::make_shared<T>(std::forward<Args>(args)...));
}


