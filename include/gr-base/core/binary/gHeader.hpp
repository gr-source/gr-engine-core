#pragma once

#include <gr-math/gmath.hpp>

#include <cstdint>
#include <cstring>
#include <array>

class Stream;

using HeaderType_ = std::uint8_t;

enum class HeaderType : HeaderType_
{
    NONE     = 0,
    METADATA = 1,
    SHADER   = 2,
    TEXTURE  = 3,
    MATERIAL = 4,
    MESH     = 5,
    OBJECT   = 6,
    SCRIPT   = 7,
    COUNT
};

constexpr uint8_t IHeaderType = static_cast<uint8_t>(HeaderType::COUNT);

static const std::array<std::string, IHeaderType> HeaderName = {
    "NONE",
    "METADATA",
    "SHADER",
    "TEXTURE",
    "MATERIAL",
    "MESH",
    "OBJECT",
    "SCRIPT"
};

#define HEADER_SCENE_SIGN       10005390867908566963ULL
#define HEADER_SCENE_EXTENSION ".scn"

#define HEADER_SHADER_SIGN      5240899614122928136ULL
#define HEADER_SHADER_EXTENSION ".gshr"

#define HEADER_MODEL_SIGN       11377574333308104762ULL
#define HEADER_MODEL_EXTENSION ".mdl"

#define HEADER_MATERIAL_SIGN    175488002581160416ULL
#define HEADER_MATERIAL_EXTENSION ".mat"

#define HEADER_MESH_SIGN        4838001395392124604ULL
#define HEADER_MESH_EXTENSION ".msh"

#define HEADER_ASSET_VERSION    65537U

typedef struct FileHeader
{
    uint64_t sign;
    uint32_t version;
} FileHeader;

typedef struct DataSegment
{
    HeaderType type;
    uint64_t size;
} DataSegment;





