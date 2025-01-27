#pragma once

#include "gr-base/core/geometry/boundbox.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/component.hpp"

class MeshFilter : public Component {
public:
    MeshFilter();

    void setMesh(Mesh *mesh);

    Mesh *getMesh();

    void processBoundbox();

    Boundbox &getBoundbox();

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;
    
private:
    Mesh *m_sharedMesh;

    Boundbox m_boundbox;
};

REGISTER_COMPONENT(MeshFilter)
