#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"

// private
TaskManager &TaskManager::GetInstance()
{
    static TaskManager instance;
    return instance;
}

TaskManager::TaskManager()
{}

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


