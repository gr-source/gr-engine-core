#include "gr-base/core/thread/taskmanager.hpp"

#include "gr-base/core/event.hpp"

// private
TaskManager &TaskManager::GetInstance()
{
    static TaskManager instance;
    return instance;
}

TaskManager::TaskManager()
{}

void TaskManager::update(std::shared_ptr<TaskInfo> info)
{
    info->task->run();

    Event::Add(&TaskManager::finish, this, info);
}

void TaskManager::finish(std::shared_ptr<TaskInfo> info)
{
    info->task->finish();

    info->finish();
}


