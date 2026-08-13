#include "gr-base/managers/renderPipelineManager.hpp"

#include "gr-base/render/pipeline/renderPipeline.hpp"

RenderPipelineManager* RenderPipelineManager::gRenderPipelineManager = nullptr;

void RenderPipelineManager::Initialize()
{
    if (gRenderPipelineManager == nullptr)
        gRenderPipelineManager = new RenderPipelineManager();
}

void RenderPipelineManager::Shutdown()
{
    if (gRenderPipelineManager != nullptr)
        delete gRenderPipelineManager;
    gRenderPipelineManager = nullptr;
}

// private
void RenderPipelineManager::set(RenderPipelineID id, std::shared_ptr<IRenderPipeline> pipeline)
{
    if (id.value >= pipelines.size())
        pipelines.resize(id.value + 1, nullptr);

    pipeline->Initialize();

    pipelines[id.value] = pipeline;
}


