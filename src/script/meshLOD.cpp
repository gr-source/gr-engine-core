#include "gr-base/script/meshLOD.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"

MeshLOD::MeshLOD()
{}

void MeshLOD::onStart()
{
    if (!HasComponent<MeshFilter>())
        return Console::LogError("[ MeshLOD ] this gameoject don't has MeshFilter!");

    if (!HasComponent<MeshRender>())
        return Console::LogError("[ MeshLOD ] this gameoject don't has MeshRender!");
}


