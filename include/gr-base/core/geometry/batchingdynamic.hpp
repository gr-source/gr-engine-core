#pragma once

// core/geomery
#include "gr-base/core/geometry/gpumesh.hpp"

// script
#include "gr-base/script/default.hpp"

class BatchingDynamic
{
public:
    BatchingDynamic(const std::vector<MaterialID> &materials, const std::vector<MeshID> &meshes);
    BatchingDynamic() = default;

    void insert(Transform *transform);

    void render();

private:
    std::vector<MaterialID> m_materials;

    std::vector<MeshID> m_meshes;

    std::vector<Transform *> m_instances;

    GpuMesh m_gpuMesh;

    bool instanced;

    void build(int id);

    void buildDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh);

    // void buildDynamicSkinnedMesh();

    void buildInstancedDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh);

    // void buildInstancedDynamicSkinnedMesh();
};



