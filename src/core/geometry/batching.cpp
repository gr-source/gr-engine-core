#include "gr-base/core/geometry/batching.hpp"

// core
#include "gr-base/core/assetsManager.hpp"

 // core, geometry
#include "gr-base/core/geometry/batchingstatic.hpp"
#include "gr-base/core/geometry/material.hpp"

// ecs
#include "gr-base/core/console.hpp"
#include "gr-base/ecs/gameObject.hpp"

// component
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

// gr-render
#include <gr-render/gVertexArray.h>

// public
Batching::Batching(std::vector<Mesh> *meshlist) : m_count(0), m_meshlist(meshlist)
{}

Batching::~Batching()
{}

void Batching::insert(Transform *transform, const std::vector<Material *> &materiallist)
{
    if (materiallist.empty())
    {
        return Console::LogError("[ Batching ] this model don't have material");
    }

    auto gameObject = transform->getGameObject();

    bool isanimated = gameObject->hasComponent<Animator>();
    bool isstatic = gameObject->getStatic();

    std::string key = "na";
    for (auto &material : materiallist)
        key += AssetsManager::GetRefMaterialToName(material);
    /*
    if (isstatic)
    {
        auto it = m_batchingstaticmap.find(key);
        if (it == m_batchingstaticmap.end())
        {
            m_batchingstaticmap.emplace(key, std::make_unique<BatchingStatic>(materiallist, m_meshlist));

            m_batchingstaticlist.push_back(m_batchingstaticmap[key].get());
        }
                
        m_batchingstaticmap[key]->insert(transform);
    } else
    {
        */
        auto it = m_batchingdynamicmap.find(key);
        if (it == m_batchingdynamicmap.end())
        {
            m_batchingdynamicmap.emplace(key, std::make_unique<BatchingDynamic>(materiallist, m_meshlist));

            m_batchingdynamiclist.push_back(m_batchingdynamicmap[key].get());
        }
                
        m_batchingdynamicmap[key]->insert(transform);
    // }

    m_count++;
}

void Batching::erase(Transform *transform)
{
    for (auto &batching : m_batchingdynamiclist)
        batching->erase(transform);

    for (auto &batching : m_batchingstaticlist)
        batching->erase(transform);
}

void Batching::render()
{
    for (auto &batching : m_batchingdynamiclist)
        batching->render();

    for (auto &batching : m_batchingstaticlist)
        batching->render();
}

/*
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
void Batching::insert(Transform *transform, const std::vector<Material *> &materialList)
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

                    m_dynamicinstancedrenderer->materialList = materialList;

                    m_dynamicinstancedrenderer->meshID = create_gpu_mesh(1);
                    create_shader(m_dynamicinstancedrenderer->meshID, materialList);
                }
                m_dynamicinstancedrenderer->transformlist.push_back(transform);
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
}

void Batching::erase(Transform *transform)
{
    if (m_firstrenderer != nullptr)
    {
        m_firstrenderer = nullptr;
    } else
    {
        auto &transformlist = m_dynamicinstancedrenderer->transformlist;

        transformlist.erase(std::remove(transformlist.begin(), transformlist.end(), transform), transformlist.end());

        if (m_drawcount <= 2)
        {
            m_firstrenderer = std::make_unique<BatchingRenderer>();
            m_firstrenderer->transform = transformlist.back();
            m_firstrenderer->materialList = m_dynamicinstancedrenderer->materialList;
            m_firstrenderer->meshID = create_gpu_mesh(0); 

            m_dynamicinstancedrenderer = nullptr;
        }
    }
    m_drawcount--;
}

void Batching::render()
{
    if (m_firstrenderer != nullptr)
    {
        auto materialist = m_firstrenderer->materialList;

        auto matrix = m_firstrenderer->transform->getGlobalMatrix();

        auto count = materialist.size();

        for (std::size_t i={};i<count;i++)
        {
            auto material = materialist[i];

            auto shader = material->getShader();

            if (m_firstrenderer->meshID == 0)
            {
                shader->setUniform("u_model", &matrix);
            }

            shader->bind();

            material->flush();

            shader->flush();

            if (m_gpuMesh[m_firstrenderer->meshID] != nullptr)
                m_gpuMesh[m_firstrenderer->meshID]->draw_mesh(i, 0);
        }

        return;
    }

    if (m_dynamicinstancedrenderer != nullptr && m_gpuMesh[m_dynamicinstancedrenderer->meshID] != nullptr)
    {
        auto &transformlist = m_dynamicinstancedrenderer->transformlist;
        auto materialist = m_dynamicinstancedrenderer->materialList;

        std::vector<Matrix4x4> matrixlist(transformlist.size(), Matrix4x4::identityMatrix);
        for (std::size_t i={};i<transformlist.size();i++)
            matrixlist[i] = transformlist[i]->getGlobalMatrix();

        auto gpuMesh = m_gpuMesh[m_dynamicinstancedrenderer->meshID].get();
        
        gpuMesh->update(matrixlist.data(), matrixlist.size());

        auto count = materialist.size();
        if (gpuMesh->m_shaderlist.size() == count)
        {
            for (std::size_t i={};i<count;i++)
            {
                if (gpuMesh->m_shaderlist[i] == nullptr)
                    continue;

                auto material = materialist[i];

                auto current = material->getShader();
                {
                    material->setShader(gpuMesh->m_shaderlist[i]);
                    auto shader = material->getShader();

                    shader->bind();

                    material->flush();

                    shader->flush();

                    gpuMesh->draw_mesh(i, matrixlist.size());
                }

                material->setShader(current);
            }
        }
    }
}

int Batching::create_gpu_mesh(int meshID)
{
    if (m_gpuMesh[meshID] != nullptr)
        return meshID;

    m_gpuMesh[meshID] = std::make_unique<GpuMesh>();
    
    build(m_gpuMesh[meshID].get(), meshID);
    return meshID;
}

void Batching::create_shader(int meshID, const std::vector<Material *> &materialList)
{
    auto count = materialList.size();

    auto gpuMesh = m_gpuMesh[meshID].get();
    gpuMesh->m_shaderlist.resize(count, nullptr);

    for (std::size_t i=0;i<count;i++)
    {
        auto material = materialList[i];

        auto shader = material->getShader();

        auto shaderparams = GetShaderParamsFromName(AssetsManager::GetShaderName(shader));
        shaderparams.flags |= ShaderOptimize_instanced;

        auto name = GetShaderParamsName(shaderparams);
        shader = AssetsManager::FindShader(name);
        if (shader == nullptr)
        {
            AssetsManager::LoadShader(shaderparams, &GpuMesh::set_shader, gpuMesh, i);
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
    
            for (int j=0;j<4;j++)
            {
                grr::gVertexArray::SetAttrib(6 + j, 4, sizeof(Matrix4x4), reinterpret_cast<const void*>(j * sizeof(Vector4)));
                grr::gVertexArray::SetAttribDivisor(6 + j, 1);
            }
        }

        meshRender->vertexArray->unbind();
    }
}

void MeshData::draw(int instanced)
{
    vertexArray->bind();

    if (pbo && instanced > 0)
    {
        grr::gVertexArray::DrawArraysInstanced(PrimitiveType::TRIANGLES, count, instanced);
    } else
    {
        grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, count);
    }
}

void MeshData::update(Matrix4x4 *data, int size)
{
    vertexArray->bind();
    grr::gVertexArray::Bind(pbo);
    grr::gVertexArray::UpdateResizeBuffer(size * sizeof(Matrix4x4), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, size * sizeof(Matrix4x4), data);
}
*/


