#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/managers/default.hpp"

#include <gr-math/types.hpp>

#include <gr-render/vertex_array.hpp>
#include <gr-render/default.hpp>

#include <vector>

struct RenderCommand
{
    Material* material;

    gr::shader* shader;

    gr::vertex_array* vao;
    gr::vertex_buffer* pbo;

    const Matrix4x4* instances;
    size_t instanceCount;

    size_t indexCount;
    size_t indexOffset;
};

struct Renderable
{
    uint64_t sortKey;
    MaterialHandle material;
    MeshHandle mesh;
    uint32_t submesh;
    const Matrix4x4 *world;
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

    static void Submit(const MaterialHandle& material, const MeshHandle& mesh, uint32_t submesh, const Matrix4x4* matrice);

private:
    static Renderer *s_Renderer;

    std::vector<RenderCommand> m_commands;

    std::vector<Renderable> m_renderables;

    std::vector<Matrix4x4> m_metrices;

    RenderTarget* m_rendertarget;

    Renderer();
    ~Renderer();

    void beginFrame();

    void endFrame();

    void render();

    void draw();

    void sorting();
};

