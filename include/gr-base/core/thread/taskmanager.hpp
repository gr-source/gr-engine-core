#pragma once

#include "threadpool.hpp"
#include "asynctask.hpp"

class TaskManager
{
public:
    template <typename R, typename T, typename ...Args>
    static std::shared_ptr<TaskInfo<R>> Add(Args &&...args);

    template <typename R, typename T>
    std::shared_ptr<TaskInfo<R>> add(std::shared_ptr<T> task);

private:
    TaskManager();
    ~TaskManager() = default;

    static TaskManager &GetInstance();

    void update(std::shared_ptr<ITask> task);

    void finish(std::shared_ptr<ITask> task);
};

// static
template <typename R, typename T, typename ...Args>
inline std::shared_ptr<TaskInfo<R>> TaskManager::Add(Args &&...args)
{
    static_assert(std::is_base_of<ITask, T>(), "This class does not inherit from ITask!");

    return GetInstance().add<R>(std::make_shared<T>(std::forward<Args>(args)...));
}

// default
template <typename R, typename T>
inline std::shared_ptr<TaskInfo<R>> TaskManager::add(std::shared_ptr<T> task)
{
    auto info = std::make_shared<TaskInfo<R>>();
    info->task = task;

    Threadpool::AddTask(info->task->priority, &TaskManager::update, this, info->task);

    return info;
}


