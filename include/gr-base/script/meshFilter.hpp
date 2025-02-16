#pragma once

#include "gr-base/core/geometry/boundbox.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/component.hpp"

class MeshFilter : public Component {
public:
    MeshFilter();

    void setMesh(std::vector<Mesh> *mesh);

    std::vector<Mesh> *getMesh();

    void processBoundbox();

    Boundbox &getBoundbox();

    bool Serialize(Stream *stream) override;

    bool Deserialize(Stream *stream) override;
    
private:
    std::vector<Mesh> *m_mesh;

    Boundbox m_boundbox;
};

REGISTER_COMPONENT(MeshFilter, false)
