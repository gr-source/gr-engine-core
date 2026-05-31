#include "gr-base/core/renderer.hpp"

#include "gr-base/core/debugdraw.hpp"
#include "gr-base/core/rendertarget.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include <algorithm>
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

    for (auto &&[material, mesh, submeshID, instances] : m_commands)
    {
        if (instances.empty())
            continue;

        bool instancing = instances.size() > 1;

        auto shader =
            material->prefab->Get(
                instancing
                    ? ShaderPrefabFlags_Instancing
                    : ShaderPrefabFlags_none);

        if (shader != material->shader)
        {
            material->shader = shader;

            material->Build();
        }

        shader->bind();

        shader->setUniform("u_mat_texture", textureID);
        shader->setUniform("u_mat_enable", enable);

        material->Flush();

        auto& submesh = mesh->submeshes[submeshID];

        GPUMesh& gpuMesh = mesh->gpuMesh;

        if (gpuMesh.vertexarray == nullptr)
            mesh->Build();

        gpuMesh.vertexarray->Bind();

        if (instancing)
        {
            auto pbo = gpuMesh.pbo;
            pbo->Bind();

            pbo->SetData(
                instances.data(),
                instances.size() * sizeof(Matrix4x4));

            glDrawElementsInstanced(
                GL_TRIANGLES,
                submesh.indexCount,
                GL_UNSIGNED_INT,
                (void*)(submesh.indexOffset * sizeof(uint32_t)),
                (GLsizei)instances.size());
        }
        else
        {
            shader->set_uniform(
                "u_mat_model",
                instances[0]);

            glDrawElements(
                GL_TRIANGLES,
                submesh.indexCount,
                GL_UNSIGNED_INT,
                (void*)(submesh.indexOffset * sizeof(uint32_t)));
        }
    }

    m_commands.clear();
}

void Renderer::SetRenderTarget(RenderTarget* rt)
{
    data.rendertarget = rt;
}

void Renderer::AddCommand(const RenderCommand& cmd)
{
    return GetInstance().addCommand(cmd);
}


void Renderer::addCommand(const RenderCommand& cmd)
{
    m_commands.push_back(cmd);
}




