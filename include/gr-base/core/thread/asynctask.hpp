#pragma once

#include <condition_variable>
#include <atomic>
#include <mutex>

#include "default.hpp"

class ITask
{
public:
    TaskPriority priority;
    
    bool finished;
    std::mutex mutex;
    std::condition_variable cv;

    std::atomic<bool> bCancelable;

    ITask(TaskPriority taskPriority) : priority(taskPriority), finished(false), bCancelable(false) {}

    virtual ~ITask() = default;

    virtual void onPreExecute() {}

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

    virtual void onPreExecute() override {}

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
    std::shared_ptr<ITask> task;

    template <typename R>
    inline R get()
    {
        IAsyncTask<R>* wrapper = static_cast<IAsyncTask<R>*>(task.get());

        std::unique_lock<std::mutex> lock(task->mutex);
        task->cv.wait(lock, [&] { return wrapper->finished; });
        return wrapper->result;
    }
};



