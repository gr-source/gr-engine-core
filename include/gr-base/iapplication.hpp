#pragma once

#include <vector>
#include <string>

class IApplication
{
public:
    static IApplication* GetInstance();

    IApplication();
    virtual ~IApplication();

    virtual void initialize() = 0;

    virtual void shutdown() = 0;

    virtual void run() = 0;

    virtual void dropPath(const std::vector<std::string>& entries) {}

private:
    static IApplication* gApplication;
};



