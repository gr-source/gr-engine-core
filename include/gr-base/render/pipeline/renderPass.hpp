#pragma once

#include "gr-base/core/assetPtr.hpp"
#include "gr-base/core/default.hpp"

#include <gr-render/default.hpp>

#include <vector>

struct RenderGraphBuild
{
    std::vector<AssetPtr<gr::texture>> write;
    std::vector<AssetPtr<gr::texture>> read;

    std::vector<AssetPtr<gr::texture>> colorAttachment;

    AssetPtr<gr::texture> depthAttachment;

    void AddColorAttachment(AssetPtr<gr::texture> texture)
    {
        colorAttachment.push_back(texture);
    }

    void SetDepthAttachment(AssetPtr<gr::texture> texture)
    {
        depthAttachment = texture;
    }

    uint64_t hash() const
    {
        constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;
        constexpr uint64_t FNV_PRIME  = 1099511628211ULL;

        uint64_t hash = FNV_OFFSET;

        auto add = [&](uint64_t value)
        {
            for (int i = 0; i < 8; ++i)
            {
                hash ^= (value >> (i * 8)) & 0xFF;
                hash *= FNV_PRIME;
            }
        };

        add(write.size());

        for (const auto& textureID : write)
            add(static_cast<TextureID>(textureID));

        add(0xDEADBEEFCAFEBABEULL);

        add(read.size());

        for (const auto& textureID : read)
            add(static_cast<TextureID>(textureID));

        return hash;
    }
};

class IRenderPass
{
public:
    uint32_t fbo = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    bool clearColor = false;
    bool clearDepth = false;

    virtual ~IRenderPass() = default;

    virtual void Setup(RenderGraphBuild& build) = 0;

    virtual void Execute() = 0;
};

