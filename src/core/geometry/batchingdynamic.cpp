#include "gr-base/core/geometry/batchingdynamic.hpp"

// core
#include "gr-base/core/assetsManager.hpp"

// core/geometry
#include "gr-base/core/geometry/material.hpp"

// script
#include "gr-base/script/transform.hpp"

// gr-render
#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

#include <algorithm>

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

bool BatchingDynamic::is_empty() const
{
    return m_instances.empty();
}

void BatchingDynamic::render()
{
    const bool useInstancing = m_instances.size() > 1;
    const std::size_t instanceCount = m_instances.size();

    if (instanceCount == 0)
        return;

    auto gpuMesh = useInstancing ? m_gpuMesh[DYNAMIC_INSTANCED_MESH].get()
                                 : m_gpuMesh[DYNAMIC_MESH].get();
    if (!gpuMesh)
        return;

    std::vector<Matrix4x4> modellist(instanceCount, Matrix4x4::identityMatrix);
    {
        for (std::size_t i={};i<instanceCount;i++)
            modellist[i] = m_instances[i]->getGlobalMatrix();

        if (useInstancing)
            gpuMesh->push(modellist.data(), modellist.size());
    }

    auto materialCount = m_materiallist.size();

    for (std::size_t i = 0; i < materialCount; ++i)
    {
        auto material = m_materiallist[i];
        auto shader = gpuMesh->getshader(i);
        if (!shader)
            continue;
    
        shader->bind();

        auto originalShader = material->getShader();
        material->setShader(shader);
        material->flush();

        if (useInstancing)
        {
            shader->flush();
            gpuMesh->draw(i, instanceCount);
        }
        else
        {
            bool enable[10] = {false};
            int textureID[10] = {0};

            for (std::size_t j={};j<10;j++)
            {
                auto texture = material->getTexture(j);
                if (texture == nullptr)
                    continue;
                
                texture->bind(i);
                enable[j] = true;
                textureID[j] = j;
            }

            shader->setUniform("u_texture", textureID);
            shader->setUniform("u_enable", enable);

            shader->setUniform("u_model", &modellist[0]);
            shader->flush();
            gpuMesh->draw(i);
        }

        material->setShader(originalShader);
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
    auto indices = mesh.getIndices();

    meshInfo.vertexArray = new grr::gVertexArray();
    meshInfo.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
    meshInfo.ebo = indices.empty() ? -1 : grr::gVertexArray::CreateBuffer(BufferType::EBO);

    meshInfo.count = vertices.size();

    meshInfo.vertexArray->bind();

    if (meshInfo.ebo != -1)
    {
        meshInfo.count = indices.size();

        grr::gVertexArray::Bind(meshInfo.ebo);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::uint32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::uint32), indices.data());
    }

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
    auto indices = mesh.getIndices();

    meshInfo.vertexArray = new grr::gVertexArray();
    meshInfo.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
    meshInfo.pbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);
    meshInfo.ebo = indices.empty() ? -1 : grr::gVertexArray::CreateBuffer(BufferType::EBO);

    meshInfo.count = vertices.size();

    meshInfo.vertexArray->bind();
    if (meshInfo.ebo != -1)
    {
        meshInfo.count = indices.size();

        grr::gVertexArray::Bind(meshInfo.ebo);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(grm::uint32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(grm::uint32), indices.data());
    }

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
