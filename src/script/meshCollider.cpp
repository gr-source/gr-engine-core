#include "gr-base/script/meshCollider.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/physics/physicsScene.hpp"

#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/console.hpp"

MeshCollider::MeshCollider() : m_triangleMesh(nullptr), m_shape(nullptr) {}

void MeshCollider::Init()
{
    auto meshFilter = GetEntity()->getComponent<MeshFilter>();

    if (meshFilter == nullptr)
        return Console::LogError("[ MeshFilter ] This %s don't has meshFilter!", GetGameObject()->getName().c_str());

    try {
        Mesh mesh_out;

        std::size_t nextIndice = 0;
        for (auto &meshID : meshFilter->getMesh())
        {
            auto mesh = AssetsManager::GetMesh(meshID);
            for (auto vertice : mesh->getVertices())
            {
                mesh_out.addVertex(vertice);
            }

            for (auto indice : mesh->getIndices())
            {
                mesh_out.addIndice(nextIndice + indice);
            }
            nextIndice += mesh->getVertices().size();
        }
         
        buildMesh(&mesh_out);
    } catch (const std::exception &e) {
        Console::LogError("[ MeshCollider ] %s",e.what());
    }
}


void MeshCollider::buildMesh(Mesh *mesh)
{
    m_triangleMesh = std::make_shared<btTriangleMesh>();

    auto indices = mesh->getIndices();
    auto vertices = mesh->getVertices();

    if (vertices.empty()) {
        throw std::runtime_error("Mesh do not has vertices");
    }

    if (!indices.empty()) {
        return buildWithIndice(indices, vertices);
    }
    buildWithVertice(vertices);
}

btCollisionShape *MeshCollider::getShape() {
    return &(*m_shape);
}

void MeshCollider::buildWithIndice(const std::vector<int> &indices, const std::vector<Vertex3D> &vertices) {
    for (uint32_t i = 0; i < indices.size(); i += 3) {
        if (i + 2 >= indices.size()) {
            return Console::LogError("[ MeshCollider ] Index access out of bounds.");
        }

        auto &vertex1 = vertices[indices[i]];
        auto &vertex2 = vertices[indices[i + 1]];
        auto &vertex3 = vertices[indices[i + 2]];

        auto trangle1 = vertex1.position;
        auto trangle2 = vertex2.position;
        auto trangle3 = vertex3.position;

        m_triangleMesh->addTriangle(
            btVector3(trangle1.x, trangle1.y, trangle1.z),
            btVector3(trangle2.x, trangle2.y, trangle2.z),
            btVector3(trangle3.x, trangle3.y, trangle3.z)
        );
    }

    return setup(GetEntity()->getComponent<Transform>());
}

void MeshCollider::buildWithVertice(const std::vector<Vertex3D> &vertices)
{
    for (size_t i = 0; i < vertices.size(); i += 3) {
        if (i + 2 >= vertices.size()) {
            throw std::runtime_error("Vertex access out of bounds.");
        }

        auto &vertex1 = vertices[i];
        auto &vertex2 = vertices[i + 1];
        auto &vertex3 = vertices[i + 2];

        auto trangle1 = vertex1.position;
        auto trangle2 = vertex2.position;
        auto trangle3 = vertex3.position;

        m_triangleMesh->addTriangle(
            btVector3(trangle1.x, trangle1.y, trangle1.z),
            btVector3(trangle2.x, trangle2.y, trangle2.z),
            btVector3(trangle3.x, trangle3.y, trangle3.z)
        );
    }

    return setup(GetEntity()->getComponent<Transform>());
}

void MeshCollider::setup(Transform *transform) {
    m_shape = std::make_shared<btBvhTriangleMeshShape>(m_triangleMesh.get(), true);
    m_shape->setLocalScaling(btVector3(transform->getLocalScale().x, transform->getLocalScale().y, transform->getLocalScale().z));
}
