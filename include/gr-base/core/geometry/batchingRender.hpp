#pragma once

#include <gr-render/gCommon.h>

#include <gr-math/gmath.hpp>

#include "default.hpp"

enum BatchingRenderFlags : uint8_t
{
    BatchingRenderFlags_Default = 1 << 0,
};

class BatchingRender
{
public:
    BatchingRender(BatchingRenderFlags flags);
    ~BatchingRender();

    void build(Mesh *mesh);

    void updatePBO(grm::size stride, const void *data);

    void render();

private:
    grr::gVertexArray *m_vertexBuffer;

    grm::u32 m_indexCount;

    grm::u32 m_vbo;

    grm::u32 m_ebo;

    grm::u32 m_pbo;

    BatchingRenderFlags m_flags;
};


