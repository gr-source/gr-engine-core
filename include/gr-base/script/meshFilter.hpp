#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/managers/default.hpp"

struct MeshFilter
{
    MeshID meshID;

    static void Serialize(Stream* stream, const MeshFilter* meshFilter);

    static void Deserialize(Stream* stream, MeshFilter* meshFilter);
};


