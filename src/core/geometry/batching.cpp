#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/geometry/boneWeight.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/ecs/gameObject.hpp"

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

void Batching::insert(Transform *transform, const std::vector<Material *> *materialList, int index)
{
    if (mInstances.find(transform) != mInstances.end())
    {
        return;
    }

    if (index == 1)
        mInstanceRender.emplace_back(transform, materialList);
    else if (index == 2 && transform->getGameObject()->hasComponent<Animator>())
        mInstanceSkinnedRender.emplace_back(transform, materialList, transform->getGameObject()->getComponent<Animator>());

    mInstances.emplace(transform);
}

void Batching::erase(Transform *transform)
{}

void Batching::setMesh(std::vector<Mesh> *meshList)
{
    auto meshCount = meshList->size();
    mMeshData[0].resize(meshCount);
    mMeshData[1].resize(meshCount);

    for (std::size_t i=0;i<meshCount;i++)
    {
        auto &mesh = (*meshList)[i];

        auto bones = mesh.getBoneWeight();

        bool bHasBone = !bones.empty();

        auto vertices = mesh.getVertices();

        auto &meshRender = mMeshData[bHasBone ? 1 : 0][i];
        meshRender.vertexArray = grr::gVertexArray::Create();
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
    {
        auto meshCount = mMeshData[0].size();

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

                mMeshData[0][i].draw();
            }
        }
    }

    {
        auto meshCount = mMeshData[1].size();

        for (auto &&[transform, materialList, animator] : mInstanceSkinnedRender)
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

                shader->setUniform("u_boneMatrices", animator->m_FinalBoneMatrices.data());
                shader->setUniform("u_model", &matrix);

                shader->bind();

                material->flush();

                shader->flush();

                mMeshData[1][i].draw();
            }
        }
    }
}

void MeshData::draw()
{
    vertexArray->bind();

    grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, count);

    vertexArray->bind();
}


