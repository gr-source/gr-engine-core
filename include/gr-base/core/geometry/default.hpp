#pragma once

#include <limits>

#include <stdint.h>

#define BATCHID_INVALID (BatchID)-1

#define ID_INVALID (uint32_t)-1

class BoneWeight;
class Vertex3D;
class Vertex2D;
class Material;
class Batching;
class Mesh;

using MaterialID = std::size_t;

typedef uint32_t BatchID;
typedef uint32_t MeshID;

constexpr BatchID InvalidBatch = std::numeric_limits<BatchID>::max();

constexpr std::size_t nill = std::numeric_limits<std::size_t>::max();

