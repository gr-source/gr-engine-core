#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <limits>
#include <stack>

class BoneWeight;
class Vertex3D;
class Vertex2D;
class Material;
class Batching;
class Mesh;

using MaterialID = std::size_t;
using MeshID = std::size_t;

constexpr std::size_t nill = std::numeric_limits<std::size_t>::max();

