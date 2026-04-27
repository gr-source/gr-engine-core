#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"

#include <iostream>

TaskManager* TaskManager::s_TaskManager = nullptr;

// public
void TaskManager::Initialize()
{
    if (s_TaskManager == nullptr)
        s_TaskManager = new TaskManager();

    logger::info("[TaskManager] Initialize");
}

void TaskManager::Shutdown()
{
    if (s_TaskManager != nullptr)
        delete s_TaskManager;

    s_TaskManager = nullptr;

    logger::info("[TaskManager] Shutdown");
}

// private
void TaskManager::update(std::shared_ptr<ITask> task)
{
    task->run();

    MainThreadDispatcher::Add(Priority::NORMAL, &TaskManager::finish, this, task);
}

void TaskManager::finish(std::shared_ptr<ITask> task)
{
    task->finish();

    {
        std::lock_guard<std::mutex> lock(task->mutex);
        task->finished = true;
    }

    task->cv.notify_all();
}


