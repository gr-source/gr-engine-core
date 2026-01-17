#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/ecs/behavior.hpp"
#include <vector>

typedef struct LODLevel
{
    MeshID meshID;
    float minDistance;
} LODLevel;

typedef struct LODMesh
{
    std::vector<LODLevel> levels;
} LODMesh;

class LODComponent : public BehaviorImpl<LODComponent>
{
public:
    std::vector<LODMesh> meshesLOD;

    bool Serialize(Stream* stream) const override;

    bool Deserialize(Stream* stream) override;
};

REGISTER_COMPONENT(LODComponent);


