#pragma once

#include "default.hpp"

#include "gr-base/core/geometry/vertex3D.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/physics/default.hpp"

#include "gr-base/ecs/entity.hpp"

class MeshCollider : public Component {
    CLASS_FUNCTIONS
public:
    MeshCollider();

    void Start() override;

    void buildMesh(Mesh *meshList);

    btCollisionShape *getShape();

private:
    std::shared_ptr<btTriangleMesh> m_triangleMesh;

    std::shared_ptr<btCollisionShape> m_shape;

    void buildWithIndice(const std::vector<int> &indices, const std::vector<Vertex3D> &vertices);

    void buildWithVertice(const std::vector<Vertex3D> &vertices);

    void setup(Transform *transform);
};

REGISTER_COMPONENT(MeshCollider, false)




