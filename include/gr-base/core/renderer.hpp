#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/managers/default.hpp"
#include "gr-base/render/default.hpp"

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

    static void SetViewport(const Rect& r);

    static void Submit(const MaterialHandle& material, const MeshHandle& mesh, uint32_t submesh, const Matrix4x4* matrice);

    static void DrawFullScreenQuad();

private:
    static Renderer *s_Renderer;

    std::vector<RenderCommand> m_commands;

    std::vector<Renderable> m_renderables;

    std::vector<Matrix4x4> m_metrices;

    Rect viewport =
        { 0.0f, 0.0f, 512.0f, 512.0f };

    struct FullScreenQuad
    {
        gr::vertex_array* vao = nullptr;
        gr::vertex_buffer* vbo = nullptr;
    } fullScreenQuad;

    Renderer();
    ~Renderer();

    void beginFrame();

    void endFrame();

    void render();

    void draw();

    void sorting();
};

