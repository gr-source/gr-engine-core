#include "gr-base/iapplication.hpp"

IApplication* IApplication::gApplication = nullptr;


IApplication* IApplication::GetInstance()
{
    return gApplication;
}

IApplication::IApplication()
{
    gApplication = this;
}

IApplication::~IApplication()
{
    gApplication = nullptr;
}



