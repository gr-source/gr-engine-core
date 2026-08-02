#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/ecs/default.hpp"

class ISystem
{
protected:
    Scene* scene = nullptr;

public:
    virtual ~ISystem() {}

    virtual void Init() {}

    virtual void Update() = 0;

    virtual void OnGUI() {}

    virtual Signature GetRead() const = 0;

    virtual Signature GetWrite() const = 0;

    friend class Scene;
};



