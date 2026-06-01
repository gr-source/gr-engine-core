#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/managers/default.hpp"

#include <gr-math/types.hpp>

#include <unordered_map>
#include <vector>

struct RenderCommand
{
    Material* material;
    Mesh* mesh;
    int subMeshID;
    std::vector<Matrix4x4> instances;
};

#include <vector>


// class RenderBatch
// {
// public:
//     std::vector<Matrix4x4> matrices;
//
//     MaterialInstance* materialInstance;
//
//     const SubMesh* cacheSubMesh;
//     Mesh* mesh;
//     int submeshID;
// };

class Renderer
{
public:
    static RenderTarget* backbuffer;

    static void Initialize();

    static void Shutdown();

    static void Process();

    static void SetRenderTarget(RenderTarget* rt);

    static void AddCommand(const RenderCommand& cmd);

private:
    std::vector<RenderCommand> m_commands;

    static void RenderCmd();
};

