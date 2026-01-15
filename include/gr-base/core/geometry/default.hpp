#pragma once

#include <limits>

#include <stdint.h>

#define BATCHID_INVALID (BatchID)-1

class BoneWeight;
class Vertex3D;
class Vertex2D;
class Material;
class Batching;
class Mesh;

using MaterialID = std::size_t;
using MeshID = std::size_t;

typedef uint32_t BatchID;

constexpr BatchID InvalidBatch = std::numeric_limits<BatchID>::max();

constexpr std::size_t nill = std::numeric_limits<std::size_t>::max();

