#include "gr-base/ecs/typeinfo.hpp"

#include "gr-base/externals/vector.h"

#include <string.h>
#include <assert.h>

TypeInfo& TypeInfo::registry_method(const char* name, MethodPointer pointer)
{
    MethodInfo info;
    info.name = strdup(name);
    info.function = pointer;

    vector_push(methods, info);

    return *this;
}


