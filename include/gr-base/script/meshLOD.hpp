#pragma once

#include "gr-base/ecs/entity.hpp"

class MeshLOD : public Component
{
    CLASS_FUNCTIONS
public:
    MeshLOD();

    void onStart() override;

    void onUpdate() override;

    void Calculate();

private:
};

REGISTER_COMPONENT(MeshLOD)


