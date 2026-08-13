#include "gr-base/render/pipeline/forwardPipeline.hpp"

#include "gr-base/managers/renderTargetManager.hpp"
#include "gr-base/render/pipeline/debugPass.hpp"
#include "gr-base/render/pipeline/scenePass.hpp"

RenderTargetID ForwardPipeline::GetOutput() const
{
    return rendertarget;
}

void ForwardPipeline::Initialize()
{
    RenderTargetDesc desc;
    desc.width = 512;
    desc.height = 512;
    desc.colorFormat = TEXTURE_FORMAT_RGBA16F;

    rendertarget =
        RenderTargetManager::Create(desc);

    graph.add<ScenePass>();
    graph.add<DebugPass>();
}

void ForwardPipeline::Execute()
{
    auto rt =
        RenderTargetManager::Get(rendertarget);

    rt->bind();

    graph.Execute();

    rt->unbind();
}

