#pragma once

#include "threadpool.hpp"
#include "asynctask.hpp"

class TaskManager
{
public:
    template <typename T, typename ...Args>
    static std::shared_ptr<TaskInfo> Add(Args &&...args);

    template <typename T>
    std::shared_ptr<TaskInfo> add(std::shared_ptr<T> task);

private:
    TaskManager();
    ~TaskManager() = default;

    static TaskManager &GetInstance();

    void update(std::shared_ptr<ITask> task);

    void finish(std::shared_ptr<ITask> task);
};

// static
template <typename T, typename ...Args>
inline std::shared_ptr<TaskInfo> TaskManager::Add(Args &&...args)
{
    static_assert(std::is_base_of<ITask, T>(), "This class does not inherit from ITask!");

    return GetInstance().add(std::make_shared<T>(std::forward<Args>(args)...));
}

// default
template <typename T>
inline std::shared_ptr<TaskInfo> TaskManager::add(std::shared_ptr<T> task)
{
    auto info = std::make_shared<TaskInfo>();
    info->task = task;

    Threadpool::AddTask(info->task->priority, &TaskManager::update, this, info->task);

    return info;
}


