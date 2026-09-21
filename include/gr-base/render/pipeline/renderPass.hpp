#pragma once

#include "gr-base/core/default.hpp"

#include <gr-render/default.hpp>

#include <vector>

struct RenderGraphBuild
{
    std::vector<TextureRef> writes;
    std::vector<TextureRef> reads;

    std::vector<TextureRef> colorAttachments;

    TextureRef depthAttachment;

    void Read(const TextureRef& texture)
    {
        reads.push_back(texture);
    }

    void Write(const TextureRef& texture)
    {
        writes.push_back(texture);
    }

    void AddColorAttachment(const TextureRef& texture)
    {
        colorAttachments.push_back(texture);
    }

    void SetDepthAttachment(const TextureRef& texture)
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

        add(writes.size());

        for (const auto& textureID : writes)
            add(static_cast<TextureID>(textureID));

        add(0xDEADBEEFCAFEBABEULL);

        add(reads.size());

        for (const auto& textureID : reads)
            add(static_cast<TextureID>(textureID));

        return hash;
    }
};

class IRenderPass
{
public:
    uint32_t fbo = 0;

    bool clearColor = false;
    bool clearDepth = false;

    virtual ~IRenderPass() = default;

    virtual void Setup(RenderGraphBuild& build) = 0;

    virtual void Execute() = 0;
};

