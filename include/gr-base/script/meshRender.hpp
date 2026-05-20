#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/managers/default.hpp"

#include <vector>

struct MeshRender
{
    std::vector<MaterialID> materials;

    static void Serialize(Stream *stream, const MeshRender* meshRender);

    static void Deserialize(Stream* stream, MeshRender* meshRender);
};




