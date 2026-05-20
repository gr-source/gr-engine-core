#pragma once

#include "gr-base/core/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"
#include "gr-base/core/materialInstance.hpp"
#include "gr-base/managers/default.hpp"
#include <gr-math/types.hpp>
#include <memory>
#include <unordered_map>

struct RenderCommand
{
    MaterialID materialID;
    MeshID meshID;
    int subMeshID;
    Matrix4x4 matrix;
};

#include <vector>


class RenderBatch
{
public:
    std::vector<Matrix4x4> matrices;

    std::shared_ptr<MaterialInstance> materialInstance;

    Mesh* mesh;
    int submeshID;
};

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

    std::unordered_map<uint64_t, size_t> m_batchMap;

    std::vector<RenderBatch> m_batches;

    void renderCmd();

    void addCommand(const RenderCommand& cmd);
};

