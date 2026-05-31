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
    std::vector<Matrix4x4> metrices;
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
    static Renderer& GetInstance();

    std::vector<RenderCommand> m_commands;

    // std::vector<RenderBatch> m_batches;
    //
    // std::vector<MaterialID> m_materiallookup;
    //
    // std::vector<MeshID> m_meshlookup;
    //
    // std::vector<int> m_submeshlookup;

    void renderCmd();

    // void sorting();

    void addCommand(const RenderCommand& cmd);
};

