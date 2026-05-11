#pragma once

#include <stdlib.h>
#include <stdint.h>

#include "default.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

typedef void (*MethodPointer)();


typedef struct ComponentSparseSet
{
    array* dense;
    ComponentID count;
    ComponentID capacity;
} ComponentSparseSet;

typedef struct ComponentInfo
{
    const char* name;
    void* vtable;
    size_t stride;
    size_t offset;
    ComponentHash hash;
    ComponentHash parent;

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
} ComponentInfo;



