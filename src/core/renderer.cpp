#include "gr-base/core/renderer.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/materialInstance.hpp"
#include "gr-base/core/rendertarget.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include <gr-render/gRender.h>
#include <gr-render/gl.h>

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

 RenderTarget* Renderer::backbuffer = nullptr;

void Renderer::Initialize()
{
    if (backbuffer == nullptr)
        backbuffer = new RenderTarget(1024, 1024);
}

void Renderer::Shutdown()
{
    if (backbuffer != nullptr)
        delete backbuffer;
    backbuffer = nullptr;
}

struct Data
{
    RenderTarget* rendertarget = nullptr;
} data;

void Renderer::Process()
{
    if (data.rendertarget == nullptr)
        return;

    data.rendertarget->bind();

    gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
    gr::gRender::SetEnable(GR_DEPTH, true);

    gr::gRender::SetBackgroundColor(Color{0.5f, 0.5f, 0.5f, 1.0f});
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_DEPTH_BUFFER | gr::GR_COLOR_BUFFER);


    gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LESS);
    GetInstance().renderCmd();
    // RenderManager::Render();

    gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LEQUAL);
    DebugDraw::Render();

    // gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LEQUAL);
    // m_skybox.render(camera);

    data.rendertarget->unbind();
}

Renderer& Renderer::GetInstance()
{
    static Renderer ins;
    return ins;
}

void Renderer::renderCmd()
{
    for (auto& batch : m_batches)
    {
        bool bInstancing = batch.matrices.size() > 1;

        auto materialInstance = batch.materialInstance;

        auto material = materialInstance->GetMaterial();

        auto shader = material->GetShaderPrefab()->Get(bInstancing ? ShaderPrefabFlags_Instancing : ShaderPrefabFlags_none);
        materialInstance->SetShader(shader);

        materialInstance->sync();

        auto mesh = batch.mesh;

        auto vertexarray = mesh->GetGpuMesh();

        bool enable[10] = {false};
        int textureID[10] = {0};

        shader->bind();
        shader->setUniform("u_mat_texture", textureID);
        shader->setUniform("u_mat_enable", enable);

        materialInstance->flush();

        vertexarray->Bind();

        auto &submesh = mesh->GetSubMesh(batch.submeshID);

        if (bInstancing)
        {
            auto pbo = mesh->GetIntancingBuffer();

            pbo->Bind();
            pbo->SetData(batch.matrices.data(), batch.matrices.size() * sizeof(Matrix4x4));
            pbo->Unbind();

            glDrawElementsInstanced(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(submesh.indexOffset * sizeof(uint32_t)), batch.matrices.size());
        } else
        {
            shader->set_uniform("u_mat_model", batch.matrices[0]);

            glDrawElements(GL_TRIANGLES, submesh.indexCount, GL_UNSIGNED_INT, (void*)(submesh.indexOffset * sizeof(uint32_t)));
        }

        batch.matrices.clear();
    }
}

void Renderer::SetRenderTarget(RenderTarget* rt)
{
    data.rendertarget = rt;
}

uint64_t make_handle(MaterialID materialID, MeshID meshID, uint32_t submeshIndex)
{
    return ((uint64_t)materialID << 32) |
           ((uint64_t)meshID << 16) |
           (uint64_t)submeshIndex;
}

void Renderer::AddCommand(const RenderCommand& cmd, MaterialID &batchID)
{
    GetInstance().addCommand(cmd, batchID);
}

void Renderer::addCommand(const RenderCommand& cmd, MaterialID &batchID)
{
    if (batchID == InvalidNode)
    {
        uint64_t key = make_handle(cmd.materialID, cmd.meshID, cmd.subMeshID);

        if (m_batchMap.find(key) != m_batchMap.end())
        {
            batchID = m_batchMap[key];
        } else
        {
            auto& batch = m_batches.emplace_back();
            batch.submeshID = cmd.subMeshID;
            batch.mesh = AssetsManager::GetMesh(cmd.meshID);
            batch.materialInstance = std::make_shared<MaterialInstance>(AssetsManager::GetMaterial(cmd.materialID));
            batch.matrices.push_back(cmd.matrix);

            m_batchMap.emplace(key, m_batches.size() - 1);

            batchID = m_batches.size() - 1;
        }
        return;
    }

    m_batches[batchID].matrices.push_back(cmd.matrix);

    /*
    uint64_t key = make_handle(cmd.materialID, cmd.meshID, cmd.subMeshID);

    if (m_batchMap.find(key) != m_batchMap.end())
    {
        m_batches[m_batchMap[key]].matrices.push_back(cmd.matrix);
    } else
    {
        auto& batch = m_batches.emplace_back();
        batch.submeshID = cmd.subMeshID;
        batch.mesh = AssetsManager::GetMesh(cmd.meshID);
        batch.materialInstance = std::make_shared<MaterialInstance>(AssetsManager::GetMaterial(cmd.materialID));
        batch.matrices.push_back(cmd.matrix);

        m_batchMap.emplace(key, m_batches.size() - 1);
    }
    */
}




