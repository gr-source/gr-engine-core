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

typedef struct FileHeader
{
    char sign[4];
    float version;
} FileHeader;

typedef struct DataSegment
{
    HeaderType type;
    uint64_t size;
} DataSegment;





