#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "default.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

typedef void (*MethodPointer)();

typedef struct ComponentInfo
{
    const char* name;
    size_t size;
    uint64_t hash;
    size_t offset;

    MethodPointer getwrappernext;
    MethodPointer getwrapperbehavior;

    union
    {
        MethodPointer constructor;
        uint64_t constructorhash;
    };

    union
    {
        MethodPointer destructor;
        uint64_t destructorhash;
    };

    union
    {
        MethodPointer start;
        uint64_t starthash;
    };

    union
    {
        MethodPointer update;
        uint64_t updatehash;
    };

    union
    {
        MethodPointer destroy;
        uint64_t destroyhash;
    };

} ComponentInfo;



