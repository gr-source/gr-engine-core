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
    ComponentID capacity;
    ComponentID count;
    array* dense;
} ComponentSparseSet;

typedef struct MethodInfo
{
    char* name;
    MethodPointer function;
} MethodInfo;

typedef struct TypeInfo
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

    MethodInfo* methods;

    TypeInfo& registry_method(const char* name, MethodPointer pointer);
} TypeInfo;



