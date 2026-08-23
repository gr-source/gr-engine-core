#pragma once

#include "gr-base/render/pipeline/default.hpp"
#include "gr-base/render/default.hpp"

#include <memory>
#include <vector>

class RenderPipelineManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T, typename... Args>
    static void Register(RenderPipelineID id, Args&&... args);

    template <typename T>
    static T* Get(RenderPipelineID id);

private:
    static RenderPipelineManager* gRenderPipelineManager;

    std::vector<std::shared_ptr<IPipeline>> pipelines;

    void set(RenderPipelineID id, std::shared_ptr<IPipeline> pipeline);
};

template <typename T, typename... Args>
void RenderPipelineManager::Register(RenderPipelineID id, Args&&... args)
{
    gRenderPipelineManager->set(id, std::make_shared<T>(std::forward<Args>(args)...));
}

template <typename T>
inline T* RenderPipelineManager::Get(RenderPipelineID id)
{
    if (id.value >= gRenderPipelineManager->pipelines.size())
        return nullptr;

    auto pipeline = gRenderPipelineManager->pipelines[id.value];

    return static_cast<T*>(pipeline != nullptr ? pipeline.get() : nullptr);
}

