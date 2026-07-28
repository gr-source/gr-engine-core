#pragma once

#include <gr-render/default.hpp>

#include <gr-render/vertex_array.hpp>

#include "gr-base/debug/debugVertex.hpp"

#ifndef DEBUG_DRAW_VERTEX_BUFFER_SIZE
    #define DEBUG_DRAW_VERTEX_BUFFER_SIZE 4096
#endif // DEBUG_DRAW_VERTEX_BUFFER_SIZE

class DebugBuffer
{
public:
    DebugBuffer();
    ~DebugBuffer();

    bool initialize();

    void drawPoint(const Vector3& position, const Color& color);

    void drawLine(const Vector3 &start, const Vector3 &end, const Color &color);

    void renderer();

private:
    DebugLine* lines;
    size_t lineCapacity;
    size_t lineCount;

    DebugPoint* points;
    size_t pointCapacity;
    size_t pointCount;

    DebugVertex vertexBuffer[DEBUG_DRAW_VERTEX_BUFFER_SIZE];
    size_t vertexCount;

    gr::vertex_array* m_vao;

    gr::vertex_buffer* m_vbo;

    gr::shader* m_shader;

    void drawLineList();

    void addLineVertex(const DebugLine& line);

    void flushDrawLine();

    void drawPointList();

    void addPointVertex(const DebugPoint& point);

    void flushDrawPoint();
};

