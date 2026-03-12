#pragma once

#include <gr-math/gmath.hpp>

#include <gr-render/vertex_array.hpp>
#include <vector>

#include "default.hpp"

class Batching
{
public:
    Batching();
    ~Batching();

    void insert(const Matrix4x4& matrix);

    void build(MeshID meshID, MaterialID materialID);

    void render();

private:
    std::shared_ptr<gr::vertex_array> m_vertexarray;

    std::shared_ptr<gr::vertex_buffer> m_pbo;

    MaterialID m_materialID;

    uint32_t m_indexCount;

    std::vector<Matrix4x4> instances;
};











