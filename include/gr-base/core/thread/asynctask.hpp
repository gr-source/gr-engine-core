#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>

#include "default.hpp"

class ITask
{
public:
    TaskPriority priority;
    
    std::mutex mutex;

    ITask(TaskPriority taskPriority) : priority(taskPriority) {}

    virtual ~ITask() = default;

    virtual void run() = 0;

    virtual void finish() = 0;
};

template <typename R>
class IAsyncTask : public ITask
{
public:
    R result;

    IAsyncTask(TaskPriority taskPriority = TaskPriority::LOW) : ITask(taskPriority) {}

    virtual ~IAsyncTask() = default;

    virtual R doInBackground() = 0;

    virtual void onPostExecute(R result) {}

private:
    void run() override;

    void finish() override;
};

template <typename R>
inline void IAsyncTask<R>::run()
{
    result = doInBackground();
}

template <typename R>
inline void IAsyncTask<R>::finish()
{
    return onPostExecute(result);
}

struct TaskInfo
{
    bool bFinish;
    std::condition_variable cv;

    std::shared_ptr<ITask> task;

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



