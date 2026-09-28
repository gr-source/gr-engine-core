#pragma once

#include "gr-base/render/pipeline/renderPass.hpp"

#include <gr-render/default.hpp>
#include <gr-render/vertex_array.hpp>
#include <gr-render/vertex_buffer.hpp>

class DebugPass : public IRenderPass
{
public:
    DebugPass(TextureRef hdr, TextureRef depth);

private:
    typedef struct DebugPoint DebugPoint;
    typedef struct DebugLine DebugLine;

    gr::vertex_buffer* m_vbo;
    gr::vertex_array* m_vao;

    gr::shader* m_shader;

    TextureRef m_hdr;

    TextureRef m_depth;

    void Setup(RenderGraphBuild& build) override;

    void Execute() override;

    void drawLineList();

    void addLineVertex(const DebugLine* line);

    void flushDrawLine();

    void drawPointList();

    void addPointVertex(const DebugPoint* point);

    void flushDrawPoint();
};

