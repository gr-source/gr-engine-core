#include "gr-base/managers/renderTargetManager.hpp"

#include <new>

RenderTargetManager* RenderTargetManager::gRenderTargetManager = nullptr;

// public
void RenderTargetManager::Initialize()
{
    if (gRenderTargetManager == nullptr)
        gRenderTargetManager = new RenderTargetManager();
}

void RenderTargetManager::Shutdown()
{
    if (gRenderTargetManager != nullptr)
        delete gRenderTargetManager;

    gRenderTargetManager = nullptr;
}

RenderTargetID RenderTargetManager::Create(const RenderTargetDesc& desc)
{
    return gRenderTargetManager->create(desc);
}

RenderTarget* RenderTargetManager::Get(RenderTargetID id)
{
    return gRenderTargetManager->sparseSet.get(id.value);
}

// private
RenderTargetID RenderTargetManager::create(const RenderTargetDesc& desc)
{
    SparseID id =
        sparseSet.emplace(desc);

    return { id };
}


