#pragma once

#include "gr-base/core/task/runtime/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include <atomic>

extern "C"
{
#include "gr-base/externals/array.h"
}

class MeshManager
{
public:
    MeshManager();

    MeshID create(const std::string& name, const Mesh* src);

    Mesh *get(MeshID id);

    void destroy(MeshID id);

    MeshID allocate();

    void deallocate(MeshID id);

    MeshID find(const std::string &name);

    const char *getname(MeshID id);

    size_t getCount() const;

private:
    std::unordered_map<MeshID, std::string> m_meshnamemap;

    std::unordered_map<std::string, MeshID> m_meshidmap;

    array* m_pool;

    MeshID m_freeID;

    MeshID m_count;

    std::mutex m_mutex;

    void realloc(size_t count);

    void addFreeID(MeshID first);
};



