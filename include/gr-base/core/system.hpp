#pragma once

#include "gr-base/core/default.hpp"

class ISystem
{
public:
    virtual ~ISystem() {}

    virtual void Init() {}

    virtual void Update(Scene* scene) = 0;

    virtual void OnGUI(Scene* scene) {}
};



