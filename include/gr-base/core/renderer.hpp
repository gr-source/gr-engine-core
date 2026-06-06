#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-math/types.hpp>

#include <vector>

struct RenderCommand
{
    Material* material;
    Mesh* mesh;
    uint32_t submesh;
    const Matrix4x4* instances;
    size_t instanceCount;
};

class Renderer
{
public:
    static void Initialize();

    static void Shutdown();

    static void BeginFrame();

    static void EndFrame();

    static void Render();

    static void SetRenderTarget(RenderTarget* rt);

    static void Submit(const RenderCommand& cmd);

private:
    static Renderer *s_Renderer;

    std::vector<RenderCommand> m_commands;

    RenderTarget* m_rendertarget;

    Renderer();
    ~Renderer();

    void beginFrame();

    void endFrame();

    void render();

    void draw();
};

