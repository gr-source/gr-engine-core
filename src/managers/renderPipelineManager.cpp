#include "gr-base/managers/renderPipelineManager.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/render/pipeline/ipipeline.hpp"

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
void RenderPipelineManager::set(RenderPipelineID id, std::shared_ptr<IPipeline> pipeline)
{
    if (id.value >= pipelines.size())
        pipelines.resize(id.value + 1, nullptr);

    try
    {
        pipeline->Initialize();
    } catch (const std::exception& e)
    {
        Console::LogError("[ RenderPipelineManager ] %s", e.what());
        return;
    }

    pipelines[id.value] = pipeline;
}


