#include "gr-base/application.hpp"

Application* Application::application = nullptr;

Application::Application()
{
    application = this;
}

Application::~Application()
{
    application = nullptr;
}



