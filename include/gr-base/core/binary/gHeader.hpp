#pragma once

#include <gr-math/gmath.hpp>

#include <cstdint>
#include <cstring>
#include <array>

class Stream;

using HeaderType_ = std::uint8_t;

typedef struct
{
    enum : HeaderType_
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
} HeaderType;

constexpr HeaderType_ IHeaderType = HeaderType::COUNT;

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

typedef struct FileHeader
{
    char sign[4];
    float version;
    uint64_t lenght;
} FileHeader;

typedef struct DataSegment
{
    HeaderType_ type;
    std::size_t sizeData;
} DataSegment;

enum gStructType {
    gStructType_None     = 1 << 0,
    gStructType_Texture  = 1 << 1,
    gStructType_Material = 1 << 2,
    gStructType_Mesh     = 1 << 3,
    gStructType_Node     = 1 << 4
};





