#pragma once

#include "gr-base/core/assetPtr.hpp"
#include "gr-base/core/default.hpp"

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
    std::unordered_map<uint64_t, uint32_t> m_cacheRenderTarget;

    std::vector<std::shared_ptr<IRenderPass>> passes;

    uint32_t GetOrCreateFrameBuffer(uint64_t hash);

    uint32_t CreateFrameBuffer(const std::vector<TextureRef>& attachments, TextureRef depth);

    void Set(std::shared_ptr<IRenderPass> pass);
};

template <typename T, typename... Args>
inline void RenderGraph::add(Args&&... args)
{
    Set(std::make_shared<T>(std::forward<Args>(args)...));
}


