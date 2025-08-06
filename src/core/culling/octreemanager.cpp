#include "gr-base/core/culling/octreemanager.hpp"
#include <gr-math/vector3.hpp>

// public
bool OctreeManager::Insert(Transform *transform)
{
    return GetInstance().insert(transform);
}

bool OctreeManager::Remove(Transform *transform)
{
    return GetInstance().remove(transform);
}

void OctreeManager::Render()
{
    return GetInstance().render();
}

// private
OctreeManager &OctreeManager::GetInstance()
{
    static OctreeManager instance;
    return instance;
}


OctreeManager::OctreeManager() : m_octree(Vector3::zero, 64.0f, 4)
{}

OctreeManager::~OctreeManager()
{}

bool OctreeManager::insert(Transform *transform)
{
    return m_octree.insert(transform);
}

bool OctreeManager::remove(Transform *transform)
{
    return m_octree.remove(transform);
}

void OctreeManager::render()
{
    return m_octree.render();
}




