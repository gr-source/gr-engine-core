#include "gr-base/managers/taskmanager.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/logger.hpp"

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
std::shared_ptr<TaskInfo> TaskManager::add(std::shared_ptr<ITask> task)
{
    auto info = std::make_shared<TaskInfo>();
    info->task = std::move(task);

    info->task->onPreExecute();

    Threadpool::Submit(info->task->priority, &TaskManager::update, this, info);

    return info;
}

void TaskManager::update(std::shared_ptr<TaskInfo> info)
{
    info->task->run();

    MainThreadDispatcher::Queue(&TaskManager::finish, this, info);
}

void TaskManager::finish(std::shared_ptr<TaskInfo> info)
{
    auto task = info->task.get();
    {
        std::lock_guard<std::mutex> lock(task->mutex);
        task->finished = true;
    }

    task->cv.notify_all();

    task->finish();
}


