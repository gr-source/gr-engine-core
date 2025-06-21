#pragma once

#include "threadpool.hpp"
#include "asynctask.hpp"

class TaskManager
{
public:
    template <class T, typename ...Args>
    static std::shared_ptr<TaskInfo> Add(Args &&...args);

    template <class T, typename ...Args>
    std::shared_ptr<TaskInfo> add(Args &&...args);

private:
    TaskManager();
    ~TaskManager() = default;

    static TaskManager &GetInstance();

    void update(std::shared_ptr<TaskInfo> info);

    void finish(std::shared_ptr<TaskInfo> info);
};

// static
template <class T, typename ...Args>
inline std::shared_ptr<TaskInfo> TaskManager::Add(Args &&...args)
{
    static_assert(std::is_base_of<ITask, T>(), "This class does not inherit from ITask!");

    return GetInstance().add<T>(std::forward<Args>(args)...);
}

// default
template <class T, typename ...Args>
inline std::shared_ptr<TaskInfo> TaskManager::add(Args &&...args)
{
    auto info = std::make_shared<TaskInfo>();
    info->task = std::make_shared<T>(std::forward<Args>(args)...);

    Threadpool::AddTask(info->task->priority, &TaskManager::update, this, info);

    return info;
}


