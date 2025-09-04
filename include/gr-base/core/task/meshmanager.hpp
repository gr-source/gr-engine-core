#pragma once

#include "gr-base/core/task/runtime/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"

class MeshManager
{
public:
    MeshManager();

    MeshID create(const std::string &name);

    void remove(MeshID id);

    Mesh &get(MeshID id);

    MeshID find(const std::string &name);

    const char *getname(MeshID id);

    std::size_t getcount();

private:
    std::unordered_map<MeshID, std::string> m_meshnamemap;

    std::unordered_map<std::string, MeshID> m_meshidmap;

    std::stack<MeshID> m_freeIDs;

    std::vector<Mesh> m_meshes;

    std::size_t m_count;

    std::mutex m_mutex;

    void addfree(MeshID id);
};



