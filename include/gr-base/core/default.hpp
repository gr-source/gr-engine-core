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

static_assert(sizeof(MaterialID) == sizeof(uint32_t),
              "MaterialID must have the same size as uint32_t");

static_assert(sizeof(TextureID) == sizeof(uint32_t),
              "TextureID must have the same size as uint32_t");

static_assert(sizeof(MeshID) == sizeof(uint32_t),
              "MeshID must have the same size as uint32_t");

typedef AssetPtr<gr::texture, TextureID> TextureRef;
typedef AssetPtr<Material, MaterialID> MaterialRef;
typedef AssetPtr<Mesh, MeshID> MeshRef;

static_assert(sizeof(TextureRef) == sizeof(uint32_t),
              "TextureRef must have the same size as uint32_t");

static_assert(sizeof(MaterialRef) == sizeof(uint32_t),
              "MaterialRef must have the same size as uint32_t");

static_assert(sizeof(MeshRef) == sizeof(uint32_t),
              "MeshRef must have the same size as uint32_t");



