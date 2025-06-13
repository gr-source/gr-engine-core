#include "gr-base/core/thread/taskmanager.hpp"

// private
TaskManager TaskManager::s_TaskManager;

// public


// private
void TaskManager::update(std::shared_ptr<TaskInfo> info)
{
    info->task->doInBackground();

    info->finish();
}

