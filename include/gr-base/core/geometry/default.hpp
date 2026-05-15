#pragma once

#include <limits>

#include <stdint.h>

#define BATCHID_INVALID (BatchID)-1

#define ID_INVALID (uint32_t)-1

class BoneWeight;
class Material;
class Batching;
class Mesh;

typedef uint32_t BatchID;

constexpr BatchID InvalidBatch = std::numeric_limits<BatchID>::max();

