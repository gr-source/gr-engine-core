#pragma once

#include "gr-base/ecs/default.hpp"

typedef void (*MethodPointer)();

typedef struct TypeInfo
{
    const char* name;
    Hash64 hash;
    TypeID index;
    size_t stride;

    bool globalCpy;
    bool serializable;

    MethodPointer constructor;
    MethodPointer destructor;
    MethodPointer copy;
    MethodPointer move;
} TypeInfo;



