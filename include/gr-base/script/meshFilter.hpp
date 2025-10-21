#pragma once

#include "gr-base/core/geometry/boundbox.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/component.hpp"

class MeshFilter : public Component
{
public:
    MeshFilter();

    void addMesh(MeshID id);

    const std::vector<MeshID> &getMesh() const;

    bool onSerialize(Stream *stream) override;

    bool onDeserialize(Stream *stream) override;
    
private:
    std::vector<MeshID> m_meshes;
};

REGISTER_COMPONENT(MeshFilter)
