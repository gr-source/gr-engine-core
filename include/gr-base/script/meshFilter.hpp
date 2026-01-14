#pragma once

#include "gr-base/core/geometry/boundbox.hpp"
#include "gr-base/core/geometry/default.hpp"

#include <gr-ecs/behavior.hpp>

class MeshFilter : public BehaviorImpl<MeshFilter>
{
public:
    MeshFilter();

    void addMesh(MeshID id);

    const std::vector<MeshID> &getMesh() const;

    bool Serialize(Stream *stream) const override;

    bool Deserialize(Stream *stream) override;
    
private:
    std::vector<MeshID> m_meshes;
};

REGISTER_COMPONENT(MeshFilter)
