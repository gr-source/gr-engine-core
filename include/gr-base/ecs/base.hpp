#pragma once

#include <cstddef>

using ScriptID = std::size_t;

static inline ScriptID GenID() noexcept
{
    static ScriptID id = 0;
    return id++;
}

template <typename T>
static inline ScriptID GenID() noexcept
{
    static ScriptID id = GenID();
    return id;
}




