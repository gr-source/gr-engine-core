#include "gr-base/core/geometry/batching.hpp"

// core
#include "gr-base/core/assetsManager.hpp"

 // core, geometry
#include "gr-base/core/geometry/material.hpp"

// ecs
#include "gr-base/core/console.hpp"
#include "gr-base/ecs/gameObject.hpp"

// component
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

// gr-render
#include <gr-render/gVertexArray.h>

// MeshInfo
void MeshInfo::push(const Matrix4x4 *data, const std::size_t &size)
{
    vertexArray->bind();
    grr::gVertexArray::Bind(pbo);
    grr::gVertexArray::UpdateResizeBuffer(size * sizeof(Matrix4x4), BufferUsage::DYNAMIC);
    grr::gVertexArray::SetBufferUpdate(0, size * sizeof(Matrix4x4), data);
}

void MeshInfo::draw(const std::size_t &instances)
{
    vertexArray->bind();
    if (instances > 0)
        return grr::gVertexArray::DrawArraysInstanced(PrimitiveType::TRIANGLES, count, instances);
    grr::gVertexArray::DrawArrays(PrimitiveType::TRIANGLES, count);
}

// gpuMesh
void GpuMesh::resize(const std::size_t &size)
{
    meshinfolist.resize(size);
    shaderlist.resize(size, nullptr);
}

MeshInfo &GpuMesh::get(const std::size_t &id)
{
    return meshinfolist[id];
}

void GpuMesh::push(const Matrix4x4 *data, const std::size_t &size)
{
    for (auto &meshinfo : meshinfolist)
        meshinfo.push(data, size);
}

void GpuMesh::draw(int id, const std::size_t &instances)
{
    meshinfolist[id].draw(instances);
}

// BatchingDynamic
BatchingDynamic::BatchingDynamic(const std::vector<Material *> &material, std::vector<Mesh> *meshlist) : m_materiallist(material), m_meshlist(meshlist)
{
    m_gpuMesh[0] = nullptr;
    m_gpuMesh[1] = nullptr;
    // build(meshlist);
}

void BatchingDynamic::insert(Transform *transform)
{
    if (m_instances.empty())
    {
        if (m_gpuMesh[DYNAMIC_MESH] == nullptr)
        {
            m_gpuMesh[DYNAMIC_MESH] = std::make_unique<GpuMesh>();

            build(m_gpuMesh[DYNAMIC_MESH].get(), m_meshlist, DYNAMIC_MESH);
        }
    } else
    {
        if (m_gpuMesh[DYNAMIC_INSTANCED_MESH] == nullptr)
        {
            m_gpuMesh[DYNAMIC_INSTANCED_MESH] = std::make_unique<GpuMesh>();

            build(m_gpuMesh[DYNAMIC_INSTANCED_MESH].get(), m_meshlist, DYNAMIC_INSTANCED_MESH);
        }
    }
    m_instances.push_back(transform);
}

void BatchingDynamic::erase(Transform *transform)
{
    m_instances.erase(std::remove(m_instances.begin(), m_instances.end(), transform), m_instances.end());
}

void BatchingDynamic::render()
{
    if (m_instances.size() <= 1)
    {
        if (m_gpuMesh[DYNAMIC_MESH] == nullptr)
            return;

        auto gpuMesh = m_gpuMesh[DYNAMIC_MESH].get();

        auto materialcount =  m_materiallist.size();

        for (std::size_t i={};i<materialcount;i++)
        {
            auto material = m_materiallist[i];

            auto current = material->getShader();
            {
                auto shader = gpuMesh->getshader(i);
                material->setShader(shader);

                material->flush();

                shader->bind();
        
                for (auto &transform : m_instances)
                {
                    auto matrix = transform->getGlobalMatrix();

                    shader->setUniform("u_model", &matrix);

                    shader->flush();

                    gpuMesh->draw(i);
                }
            }
            material->setShader(current);
        }
    } else
    {
        if (m_gpuMesh[DYNAMIC_INSTANCED_MESH] == nullptr)
            return;


        std::vector<Matrix4x4> modellist;
        for (auto &transform : m_instances)
        {
            modellist.push_back(transform->getGlobalMatrix());
        }

        auto gpuMesh = m_gpuMesh[DYNAMIC_INSTANCED_MESH].get();
        gpuMesh->push(modellist.data(), modellist.size());

        auto materialcount =  m_materiallist.size();

        for (std::size_t i={};i<materialcount;i++)
        {
            auto material = m_materiallist[i];
            
            auto current = material->getShader();
            {
                auto shader = gpuMesh->getshader(i);
                if (shader == nullptr)
                    continue;

                material->setShader(shader);
                
                material->flush();

                shader->bind();                

                shader->flush();

                gpuMesh->draw(i, modellist.size());
            }
            material->setShader(current);
        }
    }
}

// private
void BatchingDynamic::build(GpuMesh *gpuMesh, std::vector<Mesh> *meshlist, int id)
{
    auto meshCount = meshlist->size();
    gpuMesh->resize(meshCount);

    for (std::size_t i=0;i<meshCount;i++)
    {
        auto &mesh = (*meshlist)[i];

        switch (id)
        {
            case DYNAMIC_MESH:
                buildDynamicMesh(gpuMesh->get(i), mesh);
                break;
            case DYNAMIC_INSTANCED_MESH:
                buildInstancedDynamicMesh(gpuMesh->get(i), mesh);
                break;
            default:
                break;
        }
    }

    auto count = m_materiallist.size();
    for (std::size_t i={};i<count;i++)
    {
        auto material = m_materiallist[i];

        auto shader = material->getShader();

        auto shadername = AssetsManager::GetShaderName(shader);

        auto shaderparams = GetShaderParamsFromName(shadername);
        switch (id)
        {       
            case DYNAMIC_MESH:
                shaderparams.flags = ShaderOptimize_dynamic;
                break;
            case DYNAMIC_INSTANCED_MESH:
                shaderparams.flags = ShaderOptimize_dynamic | ShaderOptimize_instanced;
                break;
            default:
                break;
        }

        shadername = GetShaderParamsName(shaderparams);

        shader = AssetsManager::FindShader(shadername);
        if (shader == nullptr)
            AssetsManager::LoadShader(shaderparams, &GpuMesh::set, gpuMesh, i);
        else
            gpuMesh->set(i, shader);
    }
}

void BatchingDynamic::buildDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh)
{
    auto vertices = mesh.getVertices();

    meshInfo.vertexArray = new grr::gVertexArray();
    meshInfo.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);

    meshInfo.count = vertices.size();

    meshInfo.vertexArray->bind();

    grr::gVertexArray::Bind(meshInfo.vbo);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));
}

void BatchingDynamic::buildInstancedDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh)
{
    auto vertices = mesh.getVertices();

    meshInfo.vertexArray = new grr::gVertexArray();
    meshInfo.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
    meshInfo.pbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);

    meshInfo.count = vertices.size();

    meshInfo.vertexArray->bind();

    grr::gVertexArray::Bind(meshInfo.vbo);
    grr::gVertexArray::UpdateResizeBuffer(vertices.size() * sizeof(Vertex3D), BufferUsage::STATIC);
    grr::gVertexArray::SetBufferUpdate(0, vertices.size() * sizeof(Vertex3D), vertices.data());

    grr::gVertexArray::SetAttrib(0, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, position)));
    grr::gVertexArray::SetAttrib(1, 3,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, normal)));
    grr::gVertexArray::SetAttrib(2, 2,  sizeof(Vertex3D), reinterpret_cast<const void*>(offsetof(Vertex3D, texCoord)));

    grr::gVertexArray::Bind(meshInfo.pbo);
    for (int j=0;j<4;j++)
    {
        grr::gVertexArray::SetAttrib(6 + j, 4, sizeof(Matrix4x4), reinterpret_cast<const void*>(j * sizeof(Vector4)));
        grr::gVertexArray::SetAttribDivisor(6 + j, 1);
    }
}

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
    if (m_count > 0)
    {
        */
        // instanced
        // if (isstatic)
        // {
            // ignore
        // } else
        {
            // if (isanimated)
                // std::cout << "Insert in (instanced, dynamic, animated)" << std::endl;
            // else
            {
                auto it = m_batchingdynamicmap.find(key);
                if (it == m_batchingdynamicmap.end())
                {
                    m_batchingdynamicmap.emplace(key, std::make_unique<BatchingDynamic>(materiallist, m_meshlist));

                    m_batchingdynamiclist.push_back(m_batchingdynamicmap[key].get());
                }
                
                m_batchingdynamicmap[key]->insert(transform);
            }
        }
    /*} else
    {
        // firts draw
        if (isstatic)
        {
            // ignore
        } else
        {
            if (isanimated)
                std::cout << "Emplace fistr draw(dynamic, animated)" << std::endl;
            else
                batchdynamic.insert(transform, material);
        }
    }
    */

    m_count++;
}

void Batching::erase(Transform *transform)
{
    for (auto &batching : m_batchingdynamiclist)
        batching->erase(transform);
}

void Batching::render()
{
    for (auto &batching : m_batchingdynamiclist)
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


