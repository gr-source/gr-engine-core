#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>

#include "default.hpp"

class AsyncTask
{
public:
    std::mutex mutex;

    TaskPriority priority;

    AsyncTask(TaskPriority taskPriority = TaskPriority::LOW) : priority(taskPriority) {}

    virtual ~AsyncTask() {}

    virtual void doInBackground() = 0;

    virtual void onPostExecute() {}
};

struct TaskInfo
{
    bool bFinish;
    std::condition_variable cv;

    std::shared_ptr<AsyncTask> task;

    TaskInfo() : bFinish(false), task(nullptr) {}

    void wait()
    {
        std::unique_lock<std::mutex> lock(task->mutex);
        cv.wait(lock, [this] { return bFinish; });
    }


    void wait_for(std::chrono::duration<int64_t> second)
    {
        std::unique_lock<std::mutex> lock(task->mutex);
        cv.wait_for(lock, second);
    }

    void finish()
    {
        std::lock_guard<std::mutex> lock(task->mutex);
        bFinish = true;

        cv.notify_all();
    }
};



