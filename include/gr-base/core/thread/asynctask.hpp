#pragma once

#include <mutex>

class AsyncTask
{
public:
    std::mutex mutex;

    virtual ~AsyncTask() {}

    virtual void doInBackground() = 0;
};


