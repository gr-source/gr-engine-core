#pragma once

#include "default.hpp"

#include "gr-base/core/geometry/vertex3D.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/physics/default.hpp"

#include "gr-base/script/collider.hpp"

#include "gr-base/ecs/entity.hpp"

class MeshCollider : public Collider
{
    CLASS_FUNCTIONS
public:
    MeshCollider();

    void Init() override;

    void buildMesh(Mesh *meshList);

private:
    std::shared_ptr<btTriangleMesh> m_triangleMesh;

    void buildWithIndice(const std::vector<int> &indices, const std::vector<Vertex3D> &vertices);

    void buildWithVertice(const std::vector<Vertex3D> &vertices);

    void setup(Transform *transform);
};

// REGISTER_COMPONENT(MeshCollider, CLASS_TO_HASH(Collider))



