#pragma once

#include <cstdint>

#include "gr-base/externals/ID.hpp"

#define INVALID_ID32 UINT32_MAX

typedef uint32_t MaterialID;
typedef uint32_t MeshID;

struct GameObject;

class MaterialManager;
class MeshManager;

class Scene;

struct TextureTag
{};

typedef ID<TextureTag, uint32_t> TextureID;



