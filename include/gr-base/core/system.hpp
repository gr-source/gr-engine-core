#pragma once

#include "gr-base/core/default.hpp"

class ISystem
{
protected:
    Scene* scene = nullptr;

public:
    virtual ~ISystem() {}

    virtual void Init() {}

    virtual void Update() = 0;

    virtual void OnGUI() {}

    friend class Scene;
};



