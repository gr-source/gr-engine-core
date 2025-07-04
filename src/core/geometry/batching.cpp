#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/geometry/boneWeight.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <algorithm>
#include <gr-render/gCommon.h>
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
    for (auto batche : mBatcheList)
    {
        batche->render();
    }
}

Batching::Batching()
{}

void Batching::insert(Transform *transform, const std::vector<Material *> *materialList, int index)
{
    if (mInstances.find(transform) != mInstances.end())
    {
        return;
    }

    /*if (index == 1)
        mInstanceSkinnedRender.emplace_back(transform, materialList, nullptr);
    else if (index == 2 && transform->getGameObject()->hasComponent<Animator>())
        mInstanceSkinnedRender.emplace_back(transform, materialList, transform->getGameObject()->getComponent<Animator>());
    */
    
    renderentitylist.push_back({transform, static_cast<Animator *>(transform->getGameObject()->getComponent<Animator>()), materialList});

    mInstances.emplace(transform, index);
}

void Batching::erase(Transform *transform)
{
    auto it = mInstances.find(transform);
    if (it == mInstances.end())
    {
        return;
    }

    renderentitylist.erase(std::remove(renderentitylist.begin(), renderentitylist.end(), transform), renderentitylist.end());

    mInstances.erase(it);
}

void Batching::setMesh(std::vector<Mesh> *meshList)
{
    auto meshCount = meshList->size();
    mMeshData[0].resize(meshCount);
    mMeshData[1].resize(meshCount);

    for (std::size_t i=0;i<meshCount;i++)
    {
        auto &mesh = (*meshList)[i];

        auto bones = mesh.getBoneWeight();

        auto vertices = mesh.getVertices();

        auto &meshRender = mMeshData[0][i];
        meshRender.vertexArray = new grr::gVertexArray();
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

    for (std::size_t i=0;i<meshCount;i++)
    {
        auto &mesh = (*meshList)[i];

        auto bones = mesh.getBoneWeight();

        bool bHasBone = !bones.empty();

        auto vertices = mesh.getVertices();

        auto &meshRender = mMeshData[1][i];
        meshRender.vertexArray = new grr::gVertexArray();
        meshRender.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
        if (bHasBone)
        {
            meshRender.abo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
        }

        meshRender.count = vertices.size();

        meshRender.vertexArray->bind();

        grr::gVertexArray::Bind(meshRender.vbo);
        grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

        grr::gVertexArray::SetAttrib(0, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
        grr::gVertexArray::SetAttrib(1, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
        grr::gVertexArray::SetAttrib(2, 2,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));

        if (bHasBone)
        {
            grr::gVertexArray::Bind(meshRender.abo);
            grr::gVertexArray::UpdateResizeBuffer(bones.size() * sizeof(BoneWeight), BufferUsage::STATIC);
            grr::gVertexArray::SetBufferUpdate(0, bones.size() * sizeof(BoneWeight), bones.data());

            grr::gVertexArray::SetAttrib(4, 4,  sizeof(BoneWeight), reinterpret_cast<const void*>(offsetof(BoneWeight, m_index)));
            grr::gVertexArray::SetAttrib(5, 4,  sizeof(BoneWeight), reinterpret_cast<const void*>(offsetof(BoneWeight, m_weight)));
        }

        meshRender.vertexArray->unbind();
    }
}

void Batching::render()
{
    for (auto &&[transform, animator, materialList] : renderentitylist)
    {
        auto &meshdata = mMeshData[animator == nullptr ? 0 : 1];
            
        if (materialList->size() != meshdata.size())
        {
            continue;
        }

        auto matrix = transform->getGlobalMatrix();

        for (std::size_t j=0;j<meshdata.size();j++)
        {
            auto material = (*materialList)[j];

            auto shader = material->getShader();

            if (animator != nullptr)
                shader->setUniform("u_boneMatrices", static_cast<Animator *>(animator)->m_FinalBoneMatrices.data());
            shader->setUniform("u_model", &matrix);

            shader->bind();

            material->flush();

            shader->flush();

            meshdata[j].draw();
        }
    }
}

void MeshData::draw()
{
    vertexArray->bind();

    grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, count);

    vertexArray->bind();
}


