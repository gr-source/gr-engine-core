#pragma once

#include <cstdint>

#include <gr-render/default.hpp>

#include "gr-base/core/assetPtr.hpp"

#include "gr-base/externals/ID.hpp"

#define INVALID_ID32 UINT32_MAX

struct GameObject;
struct Mesh;

class MaterialManager;
class MeshManager;

class Material;

class Scene;

struct MaterialTag
{};

struct TextureTag
{};

struct MeshTag
{};

typedef ID<MaterialTag, uint32_t> MaterialID;
typedef ID<TextureTag, uint32_t> TextureID;
typedef ID<MeshTag, uint32_t> MeshID;

typedef AssetPtr<gr::texture, TextureID> TextureRef;
typedef AssetPtr<Material, MaterialID> MaterialRef;
typedef AssetPtr<Mesh, MeshID> MeshRef;



