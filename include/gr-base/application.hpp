#pragma once

#include <vector>
#include <string>

class Application
{
public:
    static Application* application;

    Application();
    virtual ~Application();

    virtual void initialize() = 0;

    virtual void shutdown() = 0;

    virtual void run() = 0;

    virtual void dropPath(const std::vector<std::string>& entries) {}
};



