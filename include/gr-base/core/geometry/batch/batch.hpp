#pragma once

#include "gr-base/script/default.hpp"

class Batch
{
public:
    virtual ~Batch() {}

    virtual void insert(Transform *transform) = 0;

    virtual void render() = 0;
};

