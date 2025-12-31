#include "gr-base/core/geometry/batchingdynamic.hpp"

// core
#include "gr-base/managers/assetsManager.hpp"

// core/geometry
#include "gr-base/core/culling/frustum.hpp"
#include "gr-base/core/geometry/material.hpp"

// script
#include "gr-base/script/transform.hpp"

// gr-render
#include <gr-render/gVertexArray.h>
#include <gr-render/gTexture.h>
#include <gr-render/gShader.h>

#include <algorithm>

// BatchingDynamic
BatchingDynamic::BatchingDynamic(const std::vector<MaterialID> &materials, const std::vector<MeshID> &meshes) : m_materials(materials), m_meshes(meshes), instanced(false)
{
}

void BatchingDynamic::insert(Transform *transform)
{
    if (m_instances.empty())
        build(DYNAMIC_MESH);
    else
    {
        if (!instanced)
        {
            build(DYNAMIC_INSTANCED_MESH);

            instanced = true;
        }
    }

    m_instances.push_back(transform);
}

void BatchingDynamic::render()
{
    const bool useInstancing = m_instances.size() > 1;
    const std::size_t instanceCount = m_instances.size();

    if (instanceCount == 0)
        return;

    std::vector<Matrix4x4> modellist(instanceCount, Matrix4x4::identityMatrix);
    {
        for (std::size_t i={};i<instanceCount;i++)
            modellist[i] = m_instances[i]->getGlobalMatrix();

        if (useInstancing)
            m_gpuMesh.push(modellist.data(), modellist.size());
    }

    auto materialCount = m_materials.size();

    for (std::size_t i = 0; i < materialCount; ++i)
    {
        auto &materialID = m_materials[i];
        
        auto &material = AssetsManager::GetMaterial(materialID);

        auto shader = m_gpuMesh.getshader(i);
        if (!shader)
            continue;
    
        shader->bind();

        auto originalShader = material.getShader();
        material.setShader(shader);
        material.flush();

        bool enable[10] = {false};
        int textureID[10] = {0};

        for (std::size_t j={};j<10;j++)
        {
            auto texture = material.getTexture(j);
            if (texture == nullptr)
                continue;
            
            texture->bind(i);
            enable[j] = true;
            textureID[j] = j;
        }

        shader->setUniform("u_texture", textureID);
        shader->setUniform("u_enable", enable);

        if (useInstancing)
        {
            shader->flush();
            m_gpuMesh.draw(i, instanceCount);
        }
        else
        {
            shader->setUniform("u_model", &modellist[0]);
            shader->flush();
            
            m_gpuMesh.draw(i);
        }

        material.setShader(originalShader);
    }
}

// private
void BatchingDynamic::build(int id)
{
    auto meshCount = m_meshes.size();
    m_gpuMesh.resize(meshCount);

    m_boundbox.min = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    m_boundbox.max = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

    for (std::size_t i=0;i<meshCount;i++)
    {
        auto &meshID = m_meshes[i];

        auto &mesh = AssetsManager::GetMesh(meshID);

        for (auto &vertice : mesh.getVertices())
        {
            m_boundbox.min = Math::min(m_boundbox.min, vertice.position);
            m_boundbox.max = Math::max(m_boundbox.max, vertice.position);
        }

        switch (id)
        {
            case DYNAMIC_MESH:
                buildDynamicMesh(m_gpuMesh.get(i), mesh);
                break;
            case DYNAMIC_INSTANCED_MESH:
                buildInstancedDynamicMesh(m_gpuMesh.get(i), mesh);
                break;
            default:
                break;
        }
    }

    auto count = m_materials.size();
    for (std::size_t i={};i<count;i++)
    {
        auto &materialID = m_materials[i];

        auto &material = AssetsManager::GetMaterial(materialID);

        auto shader = material.getShader();

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
            AssetsManager::LoadShader(shaderparams, &GpuMesh::set, &m_gpuMesh, i);
        else
            m_gpuMesh.set(i, shader);
    }
}

void BatchingDynamic::buildDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh)
{
    auto vertices = mesh.getVertices();
    auto indices = mesh.getIndices();

    if (meshInfo.vertexArray == nullptr)
        meshInfo.vertexArray = new grr::gVertexArray();

    if (meshInfo.vbo == -1)
        meshInfo.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);

    if (meshInfo.ebo == -1)
        meshInfo.ebo = indices.empty() ? -1 : grr::gVertexArray::CreateBuffer(BufferType::EBO);
    
    if (meshInfo.pbo != -1)
    {
        grr::gVertexArray::DeleteBuffer(meshInfo.pbo);
        meshInfo.pbo = -1;
    }

    meshInfo.count = vertices.size();

    meshInfo.vertexArray->bind();

    if (meshInfo.ebo != -1)
    {
        meshInfo.count = indices.size();

        grr::gVertexArray::Bind(meshInfo.ebo);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(u32), indices.data());
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
    
    if (meshInfo.vertexArray == nullptr)
        meshInfo.vertexArray = new grr::gVertexArray();

    if (meshInfo.vbo == -1)
        meshInfo.vbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);

    if (meshInfo.ebo == -1)
        meshInfo.ebo = indices.empty() ? -1 : grr::gVertexArray::CreateBuffer(BufferType::EBO);
    
    if (meshInfo.pbo == -1)
        meshInfo.pbo = grr::gVertexArray::CreateBuffer(BufferType::VBO);

    meshInfo.count = vertices.size();

    meshInfo.vertexArray->bind();
    if (meshInfo.ebo != -1)
    {
        meshInfo.count = indices.size();

        grr::gVertexArray::Bind(meshInfo.ebo);
        grr::gVertexArray::UpdateResizeBuffer(indices.size() * sizeof(u32), BufferUsage::STATIC);
        grr::gVertexArray::SetBufferUpdate(0, indices.size() * sizeof(u32), indices.data());
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
