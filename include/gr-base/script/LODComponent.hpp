#pragma once

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/ecs/behavior.hpp"
#include <vector>

typedef struct LODMesh
{
    std::vector<MeshID> meshes;
    float minDistance;
} LODMesh;

class LODComponent : public BehaviorImpl<LODComponent>
{
public:
    std::vector<LODMesh> meshesLOD;

    bool Serialize(Stream* stream) const override;

    bool Deserialize(Stream* stream) override;
};

REGISTER_COMPONENT(LODComponent, (uint64_t)-1);


