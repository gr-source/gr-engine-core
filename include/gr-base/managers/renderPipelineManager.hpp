#pragma once

#include "gr-base/render/pipeline/default.hpp"
#include "gr-base/render/default.hpp"

#include "gr-base/render/pipeline/pipelineID.hpp"

#include <memory>
#include <vector>

class RenderPipelineManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T, typename... Args>
    static void Register(PipelineID id, Args&&... args);

    template <typename T>
    static T* Get(PipelineID id);

private:
    static RenderPipelineManager* gRenderPipelineManager;

    std::vector<std::shared_ptr<IPipeline>> pipelines;

    void set(PipelineID id, std::shared_ptr<IPipeline> pipeline);
};

template <typename T, typename... Args>
void RenderPipelineManager::Register(PipelineID id, Args&&... args)
{
    gRenderPipelineManager->set(id, std::make_shared<T>(std::forward<Args>(args)...));
}

template <typename T>
inline T* RenderPipelineManager::Get(PipelineID id)
{
    if (id.value >= gRenderPipelineManager->pipelines.size())
        return nullptr;

    auto pipeline = gRenderPipelineManager->pipelines[id];

    return static_cast<T*>(pipeline != nullptr ? pipeline.get() : nullptr);
}

