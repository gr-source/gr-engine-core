#pragma once

// core/geomery
#include "gr-base/core/geometry/gpumesh.hpp"

// script
#include "gr-base/script/default.hpp"

class BatchingDynamic
{
public:
    BatchingDynamic(const std::vector<Material *> &materiallist, std::vector<Mesh> *meshlist);

    void insert(Transform *transform);

    void erase(Transform *transform);

    void render();

private:
    std::vector<Material *> m_materiallist;

    std::vector<Mesh> *m_meshlist;

    std::vector<Transform *> m_instances;

    std::unique_ptr<GpuMesh> m_gpuMesh[2];

    void build(GpuMesh *gpuMesh, std::vector<Mesh> *meshlist, int id);

    void buildDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh);

    // void buildDynamicSkinnedMesh();

    void buildInstancedDynamicMesh(MeshInfo &meshInfo, const Mesh &mesh);

    // void buildInstancedDynamicSkinnedMesh();
};



