#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/script/transform.hpp"

#include <gr-render/gShader.h>

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::mBatcheInstances;

std::vector<Batching *> Batching::mBatcheList;

Batching *Batching::Create(const std::string &name)
{
    mBatcheInstances.emplace(name, std::make_unique<Batching>());

    mBatcheList.push_back(&(*mBatcheInstances[name]));

    return mBatcheList.back();
}

Batching *Batching::Find(const std::string &name)
{
    auto it = mBatcheInstances.find(name);
    if (it == mBatcheInstances.end())
    {
        return nullptr;
    }
    return &(*it->second);
}

void Batching::Render()
{
    if (mBatcheList.empty())
    {
        return;
    }

    for (auto batche : mBatcheList)
    {
        batche->render();
    }
}

Batching::Batching()
{}

void Batching::insert(Transform *transform, const std::vector<Material *> *materialList)
{
    if (mInstances.find(transform) != mInstances.end())
    {
        return;
    }

    mInstanceRender.emplace_back(transform, materialList);

    mInstances.emplace(transform);
}

void Batching::erase(Transform *transform)
{}

void Batching::setMesh(std::vector<Mesh> *meshList)
{
    auto meshCount = meshList->size();

    mMeshData.resize(meshCount);

    for (std::size_t i=0;i<meshCount;i++)
    {
        auto &mesh = (*meshList)[i];

        auto vertices = mesh.getVertices();

        auto &meshRender = mMeshData[i];
        meshRender.vertexArray = grr::gVertexArray::Create();
        meshRender.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
        meshRender.count = vertices.size();

        meshRender.vertexArray->bind();

        grr::gVertexArray::Bind(meshRender.vbo);
        grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

        grr::gVertexArray::SetAttrib(0, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
        grr::gVertexArray::SetAttrib(1, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
        grr::gVertexArray::SetAttrib(2, 2,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));

        meshRender.vertexArray->unbind();
    }
}

void Batching::render()
{
    auto meshCount = mMeshData.size();

    for (auto &&[transform, materialList] : mInstanceRender)
    {
        if (materialList->size() != meshCount)
        {
            continue;
        }

        auto matrix = transform->getGlobalMatrix();

        for (std::size_t i=0;i<meshCount;i++)
        {
            auto material = (*materialList)[i];

            auto shader = material->getShader();
            if (shader == nullptr)
            {
                continue;
            }

            shader->setUniform("u_model", &matrix);

            shader->bind();

            material->flush();

            shader->flush();

            mMeshData[i].draw();
        }
    }
}

void MeshData::draw()
{
    vertexArray->bind();

    grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, count);

    vertexArray->bind();
}


