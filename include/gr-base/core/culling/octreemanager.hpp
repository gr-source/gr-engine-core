#pragma once

#include "gr-base/core/culling/octree.hpp"

#include "gr-base/script/default.hpp"

class OctreeManager
{
public:
    static bool Insert(Transform *transform);

    static bool Remove(Transform *transform);

    static void Render();

private:
    static OctreeManager &GetInstance();

    OctreeManager();
    ~OctreeManager();

    Octree m_octree;

    bool insert(Transform *transform);

    bool remove(Transform *transform);

    void render();
};


