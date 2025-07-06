#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/geometry/boneWeight.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <algorithm>
#include <gr-math/matrix4x4.hpp>
#include <gr-render/gCommon.h>
#include <gr-render/gShader.h>
#include <memory>
#include <stdexcept>

std::unordered_map<std::string, std::unique_ptr<Batching>> Batching::mBatcheInstances;

std::vector<Batching *> Batching::mBatcheList;

Batching *Batching::Create(const std::string &name, std::vector<Mesh> *meshList)
{
    mBatcheInstances.emplace(name, std::make_unique<Batching>(meshList));

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

Batching::Batching(std::vector<Mesh> *meshList) : m_drawcount(0), m_firstrenderer(nullptr), m_meshlist(meshList)
{
    m_gpuMesh[0] = nullptr;
    m_gpuMesh[1] = nullptr;
    m_gpuMesh[2] = nullptr;
}

// 0 - dynamic
// 1 - dynamic, instanced
void Batching::insert(Transform *transform, const std::vector<Material *> *materialList)
{
    auto gameObject = transform->getGameObject();

    if (m_drawcount >= 1)
    {
        if (gameObject->getStatic())
        {
            if (gameObject->hasComponent<Animator>())
            {
                std::cout << "MoveTo Instancied, Static and animated batch: " << transform->getGameObject()->getName() << std::endl;
            } else
            {
                std::cout << "MoveTo Instancied, Static, batch: " << transform->getGameObject()->getName() << std::endl;
            }

            goto end;
        } else
        {
            if (gameObject->hasComponent<Animator>())
            {
                std::cout << "MoveTo Instancied, dynamic and animated batch: " << transform->getGameObject()->getName() << std::endl;
            } else
            {
                if (m_dynamicinstancedrenderer == nullptr)
                {
                    m_dynamicinstancedrenderer = std::make_unique<BatchingRendererInstanced>();
                    if (m_firstrenderer != nullptr)
                    {
                        m_dynamicinstancedrenderer->transformlist.push_back(m_firstrenderer->transform);
                        m_firstrenderer = nullptr;
                    }

                    m_dynamicinstancedrenderer->meshID = create_gpu_mesh(1);
                    create_shader(m_dynamicinstancedrenderer->meshID, materialList);
                }
                m_dynamicinstancedrenderer->transformlist.push_back(transform);
                m_dynamicinstancedrenderer->materialList = materialList;
            }

            goto end;
        }
    }


    m_firstrenderer = std::make_unique<BatchingRenderer>();
    m_firstrenderer->transform = transform;
    m_firstrenderer->materialList = materialList;

    if (gameObject->getStatic())
    {
        // m_firstrenderer->meshID = create_gpu_mesh(gameObject->hasComponent<Animator>() ? MESHID_STATIC | MESHID_ANIMATED : MESHID_STATIC);
        goto end;
    }

    m_firstrenderer->meshID = create_gpu_mesh(0); 

end:
    m_drawcount++;

    std::cout << m_drawcount << std::endl;
}

void Batching::erase(Transform *transform)
{
    std::cout << "Destroy: " << transform->getGameObject()->getName() << std::endl;

    m_drawcount--;
}

// void Batching::setMesh(std::vector<Mesh> *meshList)
// {
    /*
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
    */
// }

void Batching::render()
{
    if (m_firstrenderer != nullptr)
    {
        auto materialist = m_firstrenderer->materialList;

        auto matrix = m_firstrenderer->transform->getGlobalMatrix();

        auto count = materialist->size();

        for (std::size_t i={};i<count;i++)
        {
            auto material = (*materialist)[i];

            auto shader = material->getShader();

            if (m_firstrenderer->meshID == 0)
            {
                shader->setUniform("u_model", &matrix);
            }

            shader->bind();

            material->flush();

            shader->flush();

            if (m_gpuMesh[m_firstrenderer->meshID] != nullptr)
                m_gpuMesh[m_firstrenderer->meshID]->draw_mesh(i);
        }

        return;
    }

    if (m_dynamicinstancedrenderer != nullptr && m_gpuMesh[m_dynamicinstancedrenderer->meshID] != nullptr)
    {
        auto materialist = m_dynamicinstancedrenderer->materialList;

        std::vector<Matrix4x4> matrixlist;
        for (auto &trabsform : m_dynamicinstancedrenderer->transformlist)
            matrixlist.push_back(trabsform->getGlobalMatrix());

        auto gpuMesh = m_gpuMesh[m_dynamicinstancedrenderer->meshID].get();
        
        // gpuMesh->update(matrixlist.data(), matrixlist.size());

        auto count = materialist->size();
        if (gpuMesh->m_shaderlist.size() == count)
        {
            for (std::size_t i={};i<count;i++)
            {
                if (gpuMesh->m_shaderlist[i] == nullptr)
                    continue;

                auto material = (*materialist)[i];

                auto shader = material->getShader();

                gpuMesh->m_shaderlist[i]->bind();

                material->flush();

                shader->flush();

                if (gpuMesh != nullptr)
                    gpuMesh->draw_mesh(i);
            }
        }
    }


    /*
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
    */
}

int Batching::create_gpu_mesh(int meshID)
{
    if (m_gpuMesh[meshID] != nullptr)
        return meshID;

    m_gpuMesh[meshID] = std::make_unique<GpuMesh>();
    
    build(m_gpuMesh[meshID].get(), meshID);
    return meshID;
}

void Batching::create_shader(int meshID, const std::vector<Material *> *materialList)
{
    auto count = materialList->size();

    auto gpuMesh = m_gpuMesh[meshID].get();
    gpuMesh->m_shaderlist.resize(count, nullptr);

    return;

    for (std::size_t i=0;i<count;i++)
    {
        auto material = (*materialList)[i];

        auto shader = material->getShader();

        auto shaderparams = GetShaderParamsFromName(AssetsManager::GetShaderName(shader));
        shaderparams.flags |= ShaderOptimize_instanced;

        auto name = GetShaderParamsName(shaderparams);
        shader = AssetsManager::FindShader(name);
        if (shader == nullptr)
        {
            AssetsManager::LoadShader(shaderparams, [&](auto current)
            {
                gpuMesh->m_shaderlist[i] = current;
            });
        }

        gpuMesh->m_shaderlist[i] = shader;
    }
}

void Batching::build(GpuMesh *gpuMesh, int meshID)
{
    auto meshCount = m_meshlist->size();
    gpuMesh->resize(meshCount);

    for (std::size_t i=0;i<meshCount;i++)
    {
        auto &mesh = (*m_meshlist)[i];

        auto bones = mesh.getBoneWeight();

        auto vertices = mesh.getVertices();

        auto meshRender = gpuMesh->get(i);
        meshRender->vertexArray = new grr::gVertexArray();
        meshRender->vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
        meshRender->pbo = meshID == 1 ? grr::gVertexArray::CreateBuffer(BufferType::VBO) : 0;

        meshRender->count = vertices.size();

        meshRender->vertexArray->bind();

        grr::gVertexArray::Bind(meshRender->vbo);
        grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

        grr::gVertexArray::SetAttrib(0, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
        grr::gVertexArray::SetAttrib(1, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
        grr::gVertexArray::SetAttrib(2, 2,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));

        if (meshID == 1)
        {
            grr::gVertexArray::Bind(meshRender->pbo);
            // grr::gVertexArray::SetAttribI(4, 1, sizeof(InstancedData), reinterpret_cast<const void*>(offsetof(InstancedData, entityID)));
            // grr::gVertexArray::SetAttribDivisor(4, 1);
    
            for (int i=0;i<4;i++) {
                grr::gVertexArray::SetAttrib(6 + i, 4, sizeof(Matrix4x4), reinterpret_cast<const void*>(sizeof(Matrix4x4) + i * sizeof(Vector4)));
                grr::gVertexArray::SetAttribDivisor(6 + i, 1);
            }
        }

        meshRender->vertexArray->unbind();
    }
}

void MeshData::draw()
{
    vertexArray->bind();

    if (pbo && instanceCount > 0)
        grr::gVertexArray::DrawArraysInstanced(PrimitiveType::TRIANGLES, count, instanceCount);
    else
        grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, count);

    vertexArray->bind();
}

void MeshData::update(Matrix4x4 *data, std::size_t size)
{
    vertexArray->bind();
    grr::gVertexArray::Bind(pbo);
    grr::gVertexArray::UpdateResizeBuffer(size * sizeof(Matrix4x4), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, size * sizeof(Matrix4x4), data);

    instanceCount = size;
}

