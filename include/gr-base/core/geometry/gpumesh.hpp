#pragma once

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gCommon.h>

struct MeshInfo
{
    grr::gVertexArray *vertexArray;

    int vbo, pbo, ebo;

    int abo;

    grm::uint32 count;

    void push(const Matrix4x4 *data, const std::size_t &size);

    void draw(const std::size_t &instances);
};

struct GpuMesh
{
    std::vector<MeshInfo> meshinfolist;

    std::vector<grr::gShader *> shaderlist;

    void resize(const std::size_t &size);

    MeshInfo &get(const std::size_t &id);

    grr::gShader *getshader(const std::size_t &id)
    {
        return shaderlist[id];
    }

    void set(int id, grr::gShader *shader)
    {
        shaderlist[id] = shader;
    }

    void push(const Matrix4x4 *data, const std::size_t &size);

    void draw(int id, const std::size_t &instances = 0);
};

#define DYNAMIC_MESH 0
#define DYNAMIC_INSTANCED_MESH 1
