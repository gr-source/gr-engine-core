#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

enum class TaskStatus {
    NONE    = 0,
    FAIL    = 1,
    SUCESS  = 2,
    PROCESS = 3
};



