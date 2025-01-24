#pragma once

#include "core/geometry/boundbox.hpp"
#include "core/geometry/default.hpp"

#include "ecs/component.hpp"

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
