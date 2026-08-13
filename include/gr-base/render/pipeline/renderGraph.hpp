#pragma once

#include "gr-base/core/assetPtr.hpp"
#include "gr-base/render/rendertarget.hpp"

#include <gr-render/default.hpp>

#include <unordered_map>
#include <memory>
#include <vector>

class IRenderPass;

class RenderGraph
{
public:
    template <typename T, typename... Args>
    void add(Args&&... args);

    void Execute();

    void compile();

private:
    std::unordered_map<Handle, RenderTarget> m_cacheRenderTarget;

    std::vector<std::tuple<RenderTarget, std::shared_ptr<IRenderPass>>> passes;

    RenderTarget Create(AssetPtr<gr::texture> color, AssetPtr<gr::texture> depth);

    void Set(std::shared_ptr<IRenderPass> pass);
};

template <typename T, typename... Args>
inline void RenderGraph::add(Args&&... args)
{
    Set(std::make_shared<T>(std::forward<Args>(args)...));
}


