#pragma once

#include <string>
#include <mutex>

class Error {
public:
    static void SetError(const std::string &msg);

    static std::string GetError();

private:
    static std::string buffer;

    static std::mutex mutex;
};



