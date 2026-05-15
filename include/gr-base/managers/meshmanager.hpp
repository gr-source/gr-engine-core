#pragma once

#include "gr-base/asset/meshasset.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/geometry/mesh.hpp"
#include "gr-base/pool/poolObject.hpp"

#include "gr-base/managers/default.hpp"

#include <unordered_map>

class MeshManager
{
public:
    MeshManager();

    MeshID create(const std::string& name, const MeshAsset* src);

    Mesh *get(MeshID id);

    void destroy(MeshID id);

    MeshID find(const std::string &name);

    const char *getname(MeshID id);

    size_t getCount() const;

private:
    std::unordered_map<MeshID, std::string> m_meshnamemap;

    std::unordered_map<std::string, MeshID> m_meshidmap;

    ObjectPool<Mesh> m_pool;

    MeshID m_count;

    void SetupMesh(Mesh* mesh, const MeshAsset* asset);

    void addFreeID(MeshID first);
};



