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
        TEXTURE  = 2,
        MATERIAL = 3,
        MESH     = 4,
        OBJECT   = 5,
        SCRIPT   = 6,
        COUNT
    };
} HeaderType;

constexpr HeaderType_ IHeaderType = HeaderType::COUNT;

static const std::array<std::string, IHeaderType> HeaderName = {
    "None",
    "Metadata",
    "Texture",
    "Material",
    "Mesh",
    "Object",
    "Script"
};

struct FileHeader {
    char signature[4];
    float version;
    size_t sizeData;
};

typedef struct
{
    HeaderType_ type;
    std::size_t sizeData;
}  DataSegment;

enum gStructType {
    gStructType_None     = 1 << 0,
    gStructType_Texture  = 1 << 1,
    gStructType_Material = 1 << 2,
    gStructType_Mesh     = 1 << 3,
    gStructType_Node     = 1 << 4
};





