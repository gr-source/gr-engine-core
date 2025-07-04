#pragma once

#include <functional>
#include <string>
#include <memory>
#include <mutex>

typedef struct TaskAsyc
{
    bool isCompleted;
    bool isWait;
} TaskAsync;

enum class TaskStatus
{
    NONE,
    PROCESS,
    SUCESS,
    FAIL
};

