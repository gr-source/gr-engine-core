#pragma once

#include "default.hpp"

#include "core/geometry/vertex3D.hpp"
#include "core/geometry/default.hpp"

#include "physics/default.hpp"

#include "ecs/entity.hpp"

class MeshCollider : public Component {
    CLASS_FUNCTIONS
public:
    MeshCollider();

    void Init() override;

    void build(Mesh *mesh);

    btCollisionShape *getShape();

private:
    std::shared_ptr<btTriangleMesh> m_triangleMesh;

    std::shared_ptr<btCollisionShape> m_shape;

    void buildWithIndice(const std::vector<int> &indices, const std::vector<Vertex3D> &vertices);

    void buildWithVertice(const std::vector<Vertex3D> &vertices);

    void setup(Transform *transform);
};

REGISTER_COMPONENT(MeshCollider)




