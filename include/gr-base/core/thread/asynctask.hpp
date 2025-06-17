#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>

class AsyncTask
{
public:
    std::mutex mutex;

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

    void finish()
    {
        std::lock_guard<std::mutex> lock(task->mutex);
        bFinish = true;

        cv.notify_one();
    }
};



