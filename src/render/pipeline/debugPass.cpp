#include "gr-base/render/pipeline/debugPass.hpp"

#include <gr-base/debug/debugDraw.hpp>

#include <gr-render/gRender.h>
#include <gr-render/gl.h>

void DebugPass::Execute()
{
    glBindVertexArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    gr::gRender::SetDepthFunc(GR_DEPTH_FUNC_LEQUAL);
    DebugDraw::Renderer();
}
