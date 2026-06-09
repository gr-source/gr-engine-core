#pragma once

#include "gr-base/core/default.hpp"

class ISystem
{
public:
    virtual ~ISystem() {}

    virtual void Update(Scene* scene) = 0;
};

// template <typename T, typename... Systems>
// class ISystemImp : public ISystem
// {};


