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
    bool enable[10] = {false};
    int textureID[10] = {0};

    gr::Shader* currentShader = nullptr;
    gr::vertex_array* currentVAO = nullptr;
    MaterialInstance* currentMaterial = nullptr;
    gr::vertex_buffer* currentInstanceBuffer = nullptr;

    for (auto& batch : m_batches)
    {
        if (batch.matrices.empty())
            continue;

        bool instancing = batch.matrices.size() > 1;

        MaterialInstance* materialInstance = batch.materialInstance;
        const Material* material = materialInstance->GetMaterial();

        gr::Shader* shader =
            material->GetShaderPrefab()->Get(
                instancing
                    ? ShaderPrefabFlags_Instancing
                    : ShaderPrefabFlags_none);

        // =========================
        // SHADER BIND CACHE
        // =========================

        if (shader != currentShader)
        {
            currentShader = shader;

            shader->bind();

            shader->setUniform("u_mat_texture", textureID);
            shader->setUniform("u_mat_enable", enable);
        }

        // =========================
        // MATERIAL CACHE
        // =========================

        if (materialInstance != currentMaterial)
        {
            currentMaterial = materialInstance;

            materialInstance->SetShader(shader);

            materialInstance->sync();
            materialInstance->flush();
        }

        // =========================
        // VAO CACHE
        // =========================

        auto vao = batch.mesh->GetGpuMesh();

        if (vao != currentVAO)
        {
            currentVAO = vao;
            vao->Bind();
        }

        // =========================
        // DRAW
        // =========================

        if (instancing)
        {
            auto pbo = batch.mesh->GetIntancingBuffer();

            if (pbo != currentInstanceBuffer)
            {
                currentInstanceBuffer = pbo;
                pbo->Bind();
            }

            pbo->SetData(
                batch.matrices.data(),
                batch.matrices.size() * sizeof(Matrix4x4));

            glDrawElementsInstanced(
                GL_TRIANGLES,
                batch.cacheSubMesh->indexCount,
                GL_UNSIGNED_INT,
                (void*)(batch.cacheSubMesh->indexOffset * sizeof(uint32_t)),
                (GLsizei)batch.matrices.size());
        }
        else
        {
            shader->set_uniform(
                "u_mat_model",
                batch.matrices[0]);

            glDrawElements(
                GL_TRIANGLES,
                batch.cacheSubMesh->indexCount,
                GL_UNSIGNED_INT,
                (void*)(batch.cacheSubMesh->indexOffset * sizeof(uint32_t)));
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

void Renderer::AddCommand(const RenderCommand& cmd)
{
    GetInstance().addCommand(cmd);
}

void Renderer::addCommand(const RenderCommand& cmd)
{
    uint64_t key = make_handle(cmd.materialID, cmd.meshID, cmd.subMeshID);

    if (m_batchMap.find(key) != m_batchMap.end())
    {
        m_batches[m_batchMap[key]].matrices.push_back(cmd.matrix);
    } else
    {
        auto& batch = m_batches.emplace_back();
        batch.submeshID = cmd.subMeshID;
        batch.mesh = cmd.mesh;
        batch.materialInstance = new MaterialInstance(AssetsManager::GetMaterial(cmd.materialID));
        batch.matrices.push_back(cmd.matrix);
        batch.mesh = cmd.mesh;
        batch.cacheSubMesh = cmd.cacheSubMesh;

        m_batchMap.emplace(key, m_batches.size() - 1);
    }
}




