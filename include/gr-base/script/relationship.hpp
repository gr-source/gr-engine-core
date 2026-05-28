#pragma once

#include "gr-base/ecs/default.hpp"

struct Relationship
{
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
    Entity prevSibling = INVALID_ENTITY;
};

