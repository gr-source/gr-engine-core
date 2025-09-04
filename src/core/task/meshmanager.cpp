#include "gr-base/core/task/meshmanager.hpp"
#include "gr-base/core/geometry/default.hpp"
#include <mutex>

MeshManager::MeshManager() : m_meshes(1024, Mesh{}), m_count(0)
{
    addfree(0);
}

MeshID MeshManager::create(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto id = m_freeIDs.top();
    m_freeIDs.pop();

    auto mesh = &m_meshes[id];
    new (mesh) Mesh();

    m_meshnamemap[id] = name;
    m_meshidmap[name] = id;

    m_count++;

    return id;
}

void MeshManager::remove(MeshID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto &mesh = m_meshes[id];
    mesh.~Mesh();

    m_freeIDs.push(id);

    auto it = m_meshnamemap.find(id);
    if (it != m_meshnamemap.end())
    {
        m_meshidmap.erase(it->second);

        m_meshnamemap.erase(id);
    }

    m_count--;
}

Mesh &MeshManager::get(MeshID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_meshes[id];
}

MeshID MeshManager::find(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_meshidmap.find(name);
    if (it == m_meshidmap.end())
        return nill;
    return it->second;
}

const char *MeshManager::getname(MeshID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_meshnamemap.find(id);
    if (it == m_meshnamemap.end())
        return nullptr;
    return it->second.c_str();
}

std::size_t MeshManager::getcount()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_count;
}

// private
void MeshManager::addfree(MeshID id)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto size = m_meshes.size() - 1;
    while (id < (size--))
    {
        m_freeIDs.push(size );
    }
}


