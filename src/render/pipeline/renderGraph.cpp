#include "gr-base/render/pipeline/renderGraph.hpp"

#include "gr-base/core/default.hpp"
#include "gr-base/render/pipeline/renderPass.hpp"

#include <gr-render/gRender.h>
#include <gr-render/texture.hpp>
#include <gr-render/color.hpp>
#include <gr-render/gl.h>

void RenderGraph::Execute()
{
    uint32_t lastFrameBuffer = -1;
    for (auto& pass : passes)
    {
        if (lastFrameBuffer != pass->fbo)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pass->fbo);

            gr::gRender::SetRenderState(gr::GR_CULL, gr::GR_BACK);
            gr::gRender::SetEnable(GR_CULL_FACE, true);
            // gr::gRender::SetEnable(GR_FRAMEBUFFER_SRGB, true);

            // gr::gRender::SetViewport(viewport.data);
            gr::gRender::SetBackgroundColor(Color::Black());

            if (pass->clearColor || pass->clearDepth)
            {
                uint32_t mask = 0;
                if (pass->clearColor)
                    mask |= gr::GR_COLOR_BUFFER;
                if (pass->clearDepth)
                    mask |= gr::GR_DEPTH_BUFFER;

                gr::gRender::SetRenderState(gr::GR_BACKGROUND, mask);
            }
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            lastFrameBuffer = pass->fbo;
        }

        pass->Execute();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#include <unordered_set>
#include <functional>
#include <algorithm>

void RenderGraph::compile()
{
    std::unordered_map<uint32_t, std::vector<std::shared_ptr<IRenderPass>>> readers;

    std::unordered_map<uint32_t, std::vector<std::shared_ptr<IRenderPass>>> writers;

    for (auto& pass : passes)
    {
        RenderGraphBuild builder;
        pass->Setup(builder);

        uint64_t hash = builder.hash();
        if (m_cacheRenderTarget.find(hash) == m_cacheRenderTarget.end())
            m_cacheRenderTarget.emplace(hash, CreateFrameBuffer(builder.colorAttachment, builder.depthAttachment));

        pass->fbo = m_cacheRenderTarget[hash];

        for (auto& read : builder.read)
        {
            readers[static_cast<uint32_t>(read)].push_back(pass);
        }

        for (auto& write : builder.write)
        {
            readers[static_cast<uint32_t>(write)].push_back(pass);
        }
    }

    std::unordered_map<std::shared_ptr<IRenderPass>, std::vector<std::shared_ptr<IRenderPass>>> graph;

    for (const auto& [component, componentReaders] : readers)
    {
        auto itWriters = writers.find(component);

        if (itWriters == writers.end())
            continue;

        const auto& componentWriters = itWriters->second;

        for (auto& reader : componentReaders)
        {
            for (auto& writer : componentWriters)
            {
                if (writer == reader)
                    continue;

                graph[reader].push_back(writer);
            }
        }
    }


    for (auto& [systemID, dependencies] : graph)
    {
        std::sort(
            dependencies.begin(),
            dependencies.end()
        );

        dependencies.erase(
            std::unique(
                dependencies.begin(),
                dependencies.end()
            ),
            dependencies.end()
        );
    }

    std::unordered_set<IRenderPass*> visited;
    std::unordered_set<IRenderPass*> temp;

    std::vector<std::shared_ptr<IRenderPass>> order;

    order.reserve(passes.size());

    bool hasCycle = false;

    std::function<void(std::shared_ptr<IRenderPass>)> dfs =
        [&](std::shared_ptr<IRenderPass> node)
    {
        if (hasCycle)
            return;

        // Encontrou um nó que ainda está sendo processado.
        // Portanto temos um ciclo.
        if (temp.count(node.get()))
        {
            hasCycle = true;
            return;
        }

        if (visited.count(node.get()))
            return;

        temp.insert(node.get());

        auto it = graph.find(node);

        if (it != graph.end())
        {
            for (auto& dependency : it->second)
            {
                dfs(dependency);

                if (hasCycle)
                    return;
            }
        }

        temp.erase(node.get());

        visited.insert(node.get());

        order.push_back(node);
    };


    for (auto& pass : passes)
    {
        if (!visited.count(pass.get()))
            dfs(pass);

        if (hasCycle)
            break;
    }

    if (hasCycle)
    {
        throw std::runtime_error(
            "Erro: ciclo detectado na DAG de sistemas!"
        );
    }

    passes = order;
}

uint32_t RenderGraph::CreateFrameBuffer(
    const std::vector<AssetPtr<gr::texture>>& attachments,
    AssetPtr<gr::texture> depth)
{
    uint32_t framebuffer = 0;

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    std::vector<GLenum> drawBuffers;
    drawBuffers.reserve(attachments.size());

    for (size_t i = 0; i < attachments.size(); ++i)
    {
        const GLenum attachment = GL_COLOR_ATTACHMENT0 + i;

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            attachment,
            GL_TEXTURE_2D,
            attachments[i]->id,
            0
        );

        drawBuffers.push_back(attachment);
    }

    if (depth)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            depth->id,
            0
        );
    }

    if (!drawBuffers.empty())
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    else
        glDrawBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("ssss");
        // tratar erro
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return framebuffer;
}

void RenderGraph::Set(std::shared_ptr<IRenderPass> pass)
{
    /*
    RenderGraphBuild builder;
    pass->Setup(builder);

    Handle sign =
        handle_create(handle_get_first(builder.color), handle_get_first(builder.depth));

    if (m_cacheRenderTarget.find(sign) == m_cacheRenderTarget.end())
    {
        RenderTarget rt =
            Create({builder.color}, {builder.depth});

        m_cacheRenderTarget[sign] = rt;
    }
    */

    passes.emplace_back(pass);
}

