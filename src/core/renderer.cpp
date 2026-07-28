#include "gr-base/core/renderer.hpp"

#include "gr-base/core/status.hpp"
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
          (uint64_t)handle_get_first(material) << 36
        | (uint64_t)handle_get_first(mesh) << 8
        | (uint64_t)submesh;
    
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
    RendererStats::InstanceCount = m_renderables.size();
    RendererStats::TriangleCount = 0;

    if (m_renderables.size() >= m_metrices.size())
        m_metrices.resize(m_renderables.size() + 1, type_traits<Matrix4x4>::identity);

    sorting();

    m_renderables.clear();

    if (m_rendertarget != nullptr)
        m_rendertarget->bind();

    gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
    gr::gRender::SetEnable(GR_DEPTH, true);
    gr::gRender::SetEnable(GR_CULL_FACE, true);

    gr::gRender::SetBackgroundColor(Color{0.5f, 0.5f, 0.5f, 1.0f});
    gr::gRender::SetRenderState(gr::GR_BACKGROUND, gr::GR_DEPTH_BUFFER | gr::GR_COLOR_BUFFER);
}

void Renderer::endFrame()
{
    if (m_rendertarget != nullptr)
        m_rendertarget->unbind();

    m_commands.clear();
}

void Renderer::render()
{
    if (m_rendertarget == nullptr)
        return;
    
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gr::gRender::SetDepthFunc(GR_DEPTH_FUNC_LESS);
    draw();

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gr::gRender::SetDepthFunc(GR_DEPTH_FUNC_LEQUAL);
    DebugDraw::Renderer();

    // gr::gRender::SetRenderState(gr::GR_DEPTH_FUNC, gr::GR_DEPTH_LEQUAL);
    // m_skybox.render(camera);
}

void Renderer::draw()
{
    Material* lastMaterial = nullptr;
    gr::Shader* lastShader = nullptr;
    gr::vertex_array* lastVao = nullptr;

    UniformID model = 0;

    for (auto &&[material, shader, vao, pbo, instances, instanceCount, indexCount, indexOffset] : m_commands)
    {
        if (vao != lastVao)
        {
            vao->Bind();
            lastVao = vao;
        }

        if (lastShader != shader)
        {
            shader->bind();

            model = shader->findUniform("u_mat_model");

            lastShader = shader;
        }

        material->shader = shader;
        material->Sync();


        // auto m = AssetsManager::GetMaterial(chunk->material);

        if (material != lastMaterial)
        {
            material->Flush();
            lastMaterial = material;
        }

        if (instanceCount > 1)
        {
            pbo->Bind();

            pbo->SetData(
                instances,
                instanceCount * sizeof(Matrix4x4));

            glDrawElementsInstanced(
                GL_TRIANGLES,
                indexCount,
                GL_UNSIGNED_INT,
                (void*)(indexOffset * sizeof(uint32_t)),
                (GLsizei)instanceCount);
        }
        else
        {
            if (model != GR_INVALID_ID)
                shader->SetUniform(
                    model,
                    instances);

            glDrawElements(
                GL_TRIANGLES,
                indexCount,
                GL_UNSIGNED_INT,
                (void*)(indexOffset * sizeof(uint32_t)));
        }

        RendererStats::TriangleCount +=
            (indexCount / 3) * instanceCount;

        // RendererStats::DrawCallCount++;
    }
}

void Renderer::sorting()
{
    std::sort(m_renderables.begin(), m_renderables.end(),
    [](const Renderable& a, const Renderable& b)
    {
        return a.sortKey < b.sortKey;
    });

    MaterialHandle lastMaterial = HANDLE_INVALID;

    MeshHandle lastMesh = HANDLE_INVALID;
    Mesh* mesh = nullptr;

    RenderCommand cmd;

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

        if (count > 0)
        {
            if (first.material != lastMaterial)
            {
                cmd.material = AssetsManager::GetMaterial(first.material);

                lastMaterial = first.material;
            }

            if (first.mesh != lastMesh)
            {
                mesh = AssetsManager::GetMesh(first.mesh);

                auto& gpuMesh = mesh->gpuMesh;

                if (gpuMesh.vertexarray == nullptr)
                    mesh->Build();

                cmd.vao = gpuMesh.vertexarray;
                cmd.pbo = gpuMesh.pbo;

                lastMesh = first.mesh;
            }

            auto& submesh = mesh->submeshes[first.submesh];

            cmd.instances = &m_metrices[matrixID - count];
            cmd.instanceCount = count;

            cmd.indexCount = submesh.indexCount;
            cmd.indexOffset = submesh.indexOffset;

            bool instancing = count > 1;

            cmd.shader =
                cmd.material->prefab->Get(
                    instancing
                        ? ShaderPrefabFlags_Instancing
                        : ShaderPrefabFlags_none);

            if (cmd.shader != nullptr)
                m_commands.push_back(cmd);
        }

        i = start;
    }

    RendererStats::BatchCount = m_commands.size();
}




