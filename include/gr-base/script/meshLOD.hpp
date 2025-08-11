#pragma once

#include "gr-base/ecs/entity.hpp"

class MeshLOD : public Component
{
    CLASS_FUNCTIONS
public:
    MeshLOD();

    void onStart();
};

REGISTER_COMPONENT(MeshLOD)


