#pragma once

#include "gr-base/pool/sparseSet.hpp"

#include "gr-base/render/rendertarget.hpp"
#include "gr-base/render/default.hpp"

class RenderTargetManager
{
public:
    static void Initialize();

    static void Shutdown();

    static RenderTargetID Create(const RenderTargetDesc& desc);

    static RenderTarget* Get(RenderTargetID id);

private:
    static RenderTargetManager* gRenderTargetManager;

    SparseSet2<RenderTarget> sparseSet;

    RenderTargetID create(const RenderTargetDesc& desc);
};


