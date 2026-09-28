#include "gr-base/core/renderer.hpp"

#include "gr-base/core/geometry/vertex2D.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/status.hpp"

#include "gr-base/managers/assetsManager.hpp"

#include <algorithm>

#include <gr-render/shader.hpp>
#include <gr-render/gl.h>
#include <iostream>

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/prefab/shaderprefab.hpp"

#include "gr-base/render/debug/debugRenderContext.hpp"

DebugRenderContext* gDebugRenderContext =
    nullptr;

Renderer* Renderer::s_Renderer = nullptr;

void Renderer::Initialize()
{
    if (s_Renderer == nullptr)
        s_Renderer = new Renderer();

    if (gDebugRenderContext == nullptr)
        gDebugRenderContext =
            new DebugRenderContext();
}

void Renderer::Shutdown()
{
    if (s_Renderer != nullptr)
        delete s_Renderer;
    s_Renderer = nullptr;

    if (gDebugRenderContext != nullptr)
    {
        if (gDebugRenderContext->lines != nullptr)
            free(gDebugRenderContext->lines);

        if (gDebugRenderContext->points != nullptr)
            free(gDebugRenderContext->points);

        delete gDebugRenderContext;
    }
    gDebugRenderContext =
        nullptr;
}

void Renderer::BeginCamera()
{
    s_Renderer->beginCamera();

    gDebugRenderContext->lineCount = 0;
    gDebugRenderContext->pointCount = 0;
}

void Renderer::EndCamera()
{
    s_Renderer->endCamera();
}

void Renderer::Render()
{
    s_Renderer->render();
}

void Renderer::Submit(
        const MaterialID& material,
        const MeshID& mesh,
        uint32_t submesh,
        const Matrix4x4* matrice)
{
    uint64_t sortKey =
          (uint64_t)material.value << 32
        | (uint64_t)mesh.value << 8
        | (uint64_t)submesh;
    
    s_Renderer->m_renderables.push_back({ sortKey, material, mesh, submesh, matrice });
}

void Renderer::DrawFullScreenQuad()
{
    s_Renderer->fullScreenQuad.vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

// priuvae
Renderer::Renderer()
{
    logger::info("[ Renderer ] Intialize '%p'.", this);

    // std::vector<Vertex2D> vertices = {
    //     {{-1.0f,  1.0f}, {0.0f, 1.0f}},
    //     {{-1.0f, -1.0f}, {0.0f, 0.0f}},
    //     {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
    //
    //     {{-1.0f,  1.0f}, {0.0f, 1.0f}},
    //     {{ 1.0f, -1.0f}, {1.0f, 0.0f}},
    //     {{ 1.0f,  1.0f}, {1.0f, 1.0f}}
    // };

    // gr::buffer_layout layout = {
    //     {gr::shader_data_type::Float2, "in_position", false},
    //     {gr::shader_data_type::Float2, "in_texCoord", false}
    // };

    // auto vbo = gr::vertex_buffer::create(vertices.data(), vertices.size() * sizeof(Vertex2D), gr::buffer_usage::static_draw);
    // vbo->SetLayout(layout);

    auto vertexarray = gr::vertex_array::create();
    // vertexarray->Bind();
    // vertexarray->AddVertexBuffer(vbo);

    // vertexarray->Unbind();

    fullScreenQuad.vao = vertexarray;
    // fullScreenQuad.vbo = vbo;
}

Renderer::~Renderer()
{
    logger::info("[ Renderer ] Shutdown '%p'.", this);
}

void Renderer::beginCamera()
{
    RendererStats::InstanceCount = 0;
    RendererStats::TriangleCount = 0;
    RendererStats::BatchCount = 0;

    m_renderables.clear();
    // m_commands.clear();
}

void Renderer::endCamera()
{
}

void Renderer::render()
{
    if (m_renderables.size() >= m_metrices.size())
        m_metrices.resize(m_renderables.size() + 1, type_traits<Matrix4x4>::identity);

    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gr::gRender::SetDepthFunc(GR_DEPTH_FUNC_LESS);
    flush();
}

void Renderer::flush()
{
    std::sort(m_renderables.begin(), m_renderables.end(),
    [](const Renderable& a, const Renderable& b)
    {
        return a.sortKey < b.sortKey;
    });

    MeshID lastMesh =
        MeshID::Invalid();

    gr::shader* lastShader =
        nullptr;

    Material* lastMaterial = nullptr;

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

        size_t instanceCount = start - i;

        if (instanceCount > 0)
        {
            auto mesh = AssetsManager::GetMesh(first.mesh);

            auto& gpuMesh = mesh->gpuMesh;

            auto& submesh = mesh->submeshes[first.submesh];

            auto material = AssetsManager::GetMaterial(first.material);
            auto instances = &m_metrices[matrixID - instanceCount];

            bool instancing = instanceCount > 1;

            ShaderVariantFlags flags = material->flags;
            if (instancing)
                flags |= ShaderVariantFlags_Instancing;
            else
                flags &= ~ShaderVariantFlags_Instancing;

            if (material->flags != flags)
            {
                material->shader = material->prefab->Get(flags);
                material->Sync();

                material->flags = flags;
            }

            auto shader =
                material->shader;

            if (lastShader != shader)
            {
                shader->bind();

                lastShader = shader;
            }


            if (material != lastMaterial)
            {
                material->Flush();
                lastMaterial = material;
            }

            if (lastMesh != first.mesh)
            {
                if (gpuMesh.vertexarray == nullptr)
                    mesh->Build();

                gpuMesh.vertexarray->Bind();

                lastMesh = first.mesh;
            }

            draw(
                    material,
                    shader,
                    gpuMesh.vertexarray,
                    gpuMesh.pbo,
                    instances,
                    instanceCount,
                    submesh.indexCount,
                    submesh.indexOffset
                );
        }

        i = start;

        RendererStats::InstanceCount += instanceCount;
        RendererStats::BatchCount++;
    }
}

void Renderer::draw(Material* material,
    gr::shader* shader,
    gr::vertex_array* vao,
    gr::vertex_buffer* pbo,
    const Matrix4x4* instances,
    size_t instanceCount,
    size_t indexCount,
    size_t indexOffset)
{

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
        shader->setUniform("u_mat_model", instances);

        glDrawElements(
            GL_TRIANGLES,
            indexCount,
            GL_UNSIGNED_INT,
            (void*)(indexOffset * sizeof(uint32_t)));
    }

    RendererStats::TriangleCount +=
        (indexCount / 3) * instanceCount;
}



