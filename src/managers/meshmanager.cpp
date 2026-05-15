#include "gr-base/managers/meshmanager.hpp"

#include "gr-base/core/geometry/default.hpp"

MeshManager::MeshManager() : m_count(0)
{}

MeshID MeshManager::create(const std::string &name, const MeshAsset* src)
{
    MeshID id = m_pool.create();

    Mesh* mesh = m_pool.get(id);
    if (src != nullptr)
        SetupMesh(mesh, src);

    m_meshnamemap[id] = name;
    m_meshidmap[name] = id;

    m_count++;

    return id;
}

Mesh *MeshManager::get(MeshID id)
{
    return m_pool.get(id);
}

void MeshManager::destroy(MeshID id)
{
    Mesh* mesh = get(id);
    if (mesh == nullptr)
        return;

    mesh->~Mesh();

    m_meshidmap.erase(m_meshnamemap[id]);
    m_meshnamemap.erase(id);

    m_count--;
}

MeshID MeshManager::find(const std::string &name)
{
    auto it = m_meshidmap.find(name);
    if (it == m_meshidmap.end())
        return ID_INVALID;
    return it->second;
}

const char *MeshManager::getname(MeshID id)
{
    auto it = m_meshnamemap.find(id);
    if (it == m_meshnamemap.end())
        return nullptr;
    return it->second.c_str();
}

size_t MeshManager::getCount() const
{
    return m_count;
}

// private
void MeshManager::SetupMesh(Mesh* mesh, const MeshAsset* asset)
{
    mesh->SetVertices(asset->vertices);

    for (auto& sub : asset->submeshes)
        mesh->addTriangles(sub.indices);
}



