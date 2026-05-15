#pragma once

#include "gr-base/core/task/runtime/default.hpp"

#include "gr-base/core/geometry/material.hpp"

#include "gr-base/core/geometry/default.hpp"

extern "C"
{
#include "gr-base/externals/array.h"
}

#include "gr-base/managers/default.hpp"

class MaterialManager
{
public:
    MaterialManager();

    MaterialID create(const std::string &name);

    Material *get(MaterialID id);

    MaterialID find(const std::string &name);

    const char *getname(MaterialID id);

    void destroy(MaterialID id);

    MaterialID GetCount() const;

private:
    std::unordered_map<std::string, MaterialID> m_materialnamemap;

    std::unordered_map<MaterialID, std::string> m_materialmap;

    array* m_pool;

    MaterialID m_count;

    MaterialID m_freeID;

    MaterialID allocate();

    void realloc(MaterialID size);

    void addFreeID(MaterialID start);
};


