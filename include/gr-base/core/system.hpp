#pragma once

#include "gr-base/core/default.hpp"

class System
{
public:
    virtual ~System() {}

    virtual void Update(Scene* scene) = 0;
};


