#include "gr-base/script/meshLOD.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/geometry/vertex3D.hpp"
#include "gr-base/script/baserender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "meshoptimizer.h"
#include <cstdint>

MeshLOD::MeshLOD()
{}

void MeshLOD::onStart()
{
    if (!HasComponent<MeshFilter>())
        return Console::LogError("[ MeshLOD ] this gameoject don't has MeshFilter!");

    if (!HasComponent<MeshRender>() || !HasComponent<SkinnedMeshRender>())
        return Console::LogError("[ MeshLOD ] this gameoject don't has MeshRender!");

    bCanUseUpdate = true;

    Calculate();
}

void MeshLOD::onUpdate()
{
    std::cout << "check" << std::endl;
}

void MeshLOD::Calculate()
{
    MeshFilter *meshfilter = GetComponent<MeshFilter>();

    auto meshlist = meshfilter->getMesh();
    if (meshlist == nullptr)
        return Console::LogError("[ MeshLOD ] this meshfikter don't has mesh!");

    auto meshlistlod = AssetsManager::CreateMesh(AssetsManager::GetMeshListRefToName(meshlist) + "_lod0");
    meshlistlod->resize(meshlist->size());

    for (std::size_t i=0;i<meshlist->size();i++)
    {
        auto vertices = (*meshlist)[i].getVertices();
        auto indices = (*meshlist)[i].getIndices();

        std::vector<uint32_t> simplified(indices.size());
        size_t target_index_count = indices.size() * 1.f;

        std::vector<int> lod_indices(indices.size(), 0);

        float error_result = 1.0f;

        size_t result_count = meshopt_simplify(
            lod_indices.data(),                // destination
            indices.data(),           // original indices
            indices.size(),           // number of indices
            reinterpret_cast<const float*>(&vertices[0].position), // pointer to positions
            vertices.size(),                   // vertex count
             sizeof(Vertex3D),                    // stride in bytes
            target_index_count,                // target LOD triangle count * 3
            1.0f,                              // target error
            meshopt_SimplifyLockBorder,        // options (or 0)
            &error_result                      // optional result error
        );

        lod_indices.resize(result_count);

        Mesh &mesh = (*meshlistlod)[i];
        for (auto &vertice : vertices)
            mesh.addVertex(vertice);

        for (auto &indice : lod_indices)
            mesh.addIndice(indice);
    }

    BaseRender *baserender = HasComponent<MeshRender>() ? static_cast<BaseRender *>(GetComponent<MeshRender>()) : static_cast<BaseRender *>(GetComponent<SkinnedMeshRender>());
    baserender->build(meshlistlod);

    Console::LogInfo("[ MeshLOD ] bake sucessfully!");
}


