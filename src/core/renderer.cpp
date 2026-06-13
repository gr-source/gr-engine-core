#include "gr-base/core/renderer.hpp"

#include "gr-base/debug/debugDraw.hpp"

#include "gr-base/core/rendertarget.hpp"
#include "gr-base/core/logger.hpp"

#include "gr-base/managers/assetsManager.hpp"
#include <algorithm>
#include <gr-render/gRender.h>
#include <gr-render/gl.h>

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

Renderer* Renderer::s_Renderer = nullptr;

void Renderer::Initialize()
{
    if (s_Renderer == nullptr)
        s_Renderer = new Renderer();
}

void Renderer::Shutdown()
{
    if (s_Renderer != nullptr)
        delete s_Renderer;
    s_Renderer = nullptr;
}

void Renderer::BeginFrame()
{
    s_Renderer->beginFrame();
}

void Renderer::EndFrame()
{
    s_Renderer->endFrame();
}

void Renderer::Render()
{
    s_Renderer->render();
}

void Renderer::SetRenderTarget(RenderTarget* rt)
{
    s_Renderer->m_rendertarget = rt;
}

void Renderer::Submit(const MaterialHandle& material, const MeshHandle& mesh, uint32_t submesh, const Matrix4x4* matrice)
{
    uint64_t sortKey =
          (uint64_t)handle_get_first(material) << 36  // Ocupa bits 36 a 63
        | (uint64_t)handle_get_first(mesh) << 8       // Ocupa bits 8 a 35
        | (uint64_t)submesh;                          // Ocupa bits 0 a 7
    
    s_Renderer->m_renderables.push_back({ sortKey, material, mesh, submesh, matrice });
}

// priuvae
Renderer::Renderer() : m_rendertarget(nullptr)
{
    logger::info("[ Renderer ] Intialize '%p'.", this);
}

Renderer::~Renderer()
{
    logger::info("[ Renderer ] Shutdown '%p'.", this);
}

void Renderer::beginFrame()
{
    if (m_renderables.size() >= m_metrices.size())
        m_metrices.resize(m_renderables.size() + 1, Matrix4x4::identityMatrix);

    sorting();

    m_renderables.clear();

    m_rendertarget->bind();

    gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
    gr::gRender::SetEnable(GR_DEPTH, true);
    gr::gRender::SetEnable(GR_CULL_FACE, true);

    gr::gRender::SetBackgroundColor(Color{0.5f, 0.5f, 0.5f, 1.0f});
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_DEPTH_BUFFER | gr::GR_COLOR_BUFFER);
}

void Renderer::endFrame()
{
    m_rendertarget->unbind();

    m_commands.clear();
}

void Renderer::render()
{
    if (m_rendertarget == nullptr)
        return;
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LESS);
    draw();

    gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LEQUAL);
    DebugDraw::Render();

    // gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LEQUAL);
    // m_skybox.render(camera);
}

void Renderer::draw()
{
    bool enable[10] = {false};
    int textureID[10] = {0};

    Material* lastMaterial = nullptr;
    gr::Shader* lastShader = nullptr;
    gr::vertex_array* lastVao = nullptr;

    UniformID model = 0;

    for (auto &&[material, mesh, submeshID, instances, instanceCount] : m_commands)
    {
        if (!instanceCount)
            continue;

        bool instancing = instanceCount > 1;

        auto shader =
            material->prefab->Get(
                instancing
                    ? ShaderPrefabFlags_Instancing
                    : ShaderPrefabFlags_none);

        if (lastShader != shader)
        {
            shader->bind();

            shader->setUniform("u_mat_texture", textureID);
            shader->setUniform("u_mat_enable", enable);

            model = shader->findUniform("u_mat_model");

            lastShader = shader;
        }

        if (shader != material->shader)
        {
            material->shader = shader;

            material->Build();
            material->Flush();
        }

        auto& submesh = mesh->submeshes[submeshID];

        GPUMesh& gpuMesh = mesh->gpuMesh;

        if (gpuMesh.vertexarray == nullptr)
            mesh->Build();

        auto vao = gpuMesh.vertexarray;
        if (vao != lastVao)
        {
            vao->Bind();
            lastVao = vao;
        }

        if (instancing)
        {
            // auto pbo = gpuMesh.pbo;
            // pbo->Bind();
            //
            // pbo->SetData(
            //     instances,
            //     instanceCount * sizeof(Matrix4x4));
            //
            // glDrawElementsInstanced(
            //     GL_TRIANGLES,
            //     submesh.indexCount,
            //     GL_UNSIGNED_INT,
            //     (void*)(submesh.indexOffset * sizeof(uint32_t)),
            //     (GLsizei)instanceCount);
        }
        else
        {
            shader->SetUniform(
                model,
                instances);

            glDrawElements(
                GL_TRIANGLES,
                submesh.indexCount,
                GL_UNSIGNED_INT,
                (void*)(submesh.indexOffset * sizeof(uint32_t)));
        }

        vao->Unbind();
    }
}

void Renderer::sorting()
{
    std::sort(m_renderables.begin(), m_renderables.end(),
    [](const Renderable& a, const Renderable& b)
    {
        return a.sortKey < b.sortKey;
    });

    size_t i = 0, matrixID = 0;
    while (i < m_renderables.size())
    {
        const auto& first = m_renderables[i];

        size_t start = i;

        while (start < m_renderables.size())
        {
            const auto& current = m_renderables[start];

            bool sameBatch = current.sortKey == first.sortKey;

            if (!sameBatch)
                break;

            m_metrices[matrixID++] = *current.world;

            ++start;
        }

        size_t count = start - i;

        RenderCommand cmd;
        cmd.material = AssetsManager::GetMaterial(first.material);
        cmd.mesh = AssetsManager::GetMesh(first.mesh);
        cmd.submesh = first.submesh;
        cmd.instances = &m_metrices[matrixID - count];
        cmd.instanceCount = count;

        m_commands.push_back(cmd);

        i = start;
    }
}




