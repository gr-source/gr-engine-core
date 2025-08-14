#include "gr-base/script/LODGroup.hpp"

#include "gr-base/core/console.hpp"

#include "gr-base/script/camera.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/transform.hpp"

#include "gr-base/ecs/gameObject.hpp"

LODGroup::LODGroup() : m_LODcount(0), m_transform(nullptr)
{}

void LODGroup::onInit()
{
    for (int i=0;i<5;i++)
        m_LODlist[i] = {0.0f, nullptr};
}

void LODGroup::onStart()
{
    m_transform = GetComponent<Transform>();

    bCanUseUpdate = true;
}

/*
void LODGroup::onUpdate()
{
    auto cam = Camera::Current();
    if (!cam)
        return;

    float dist = Math::distance(cam->getPosition(), m_transform->getPosition());

    const GameObject* selectedLOD = nullptr;
    float minDistance = std::numeric_limits<float>::max();

    for (int i = 0; i < m_LODcount; ++i)
    {
        const auto& lod = m_LODlist[i];
        if (dist < lod.distance && lod.distance < minDistance)
        {
            selectedLOD = lod.level;
            minDistance = lod.distance;
        }
    }

    if (selectedLOD)
    {
        for (int i = 0; i < m_LODcount; ++i)
        {
            if (m_LODlist[i].level)
            {
                m_LODlist[i].level->setActive(m_LODlist[i].level == selectedLOD);
            }
        }
    }
}
*/

int LODGroup::GetLODCount() const
{
    return m_LODcount;
}

int LODGroup::AddLOD(float distance, GameObject *level)
{
    int id = m_LODcount;
    auto &lod = m_LODlist[id];
    lod.distance = distance;
    lod.level = level;

    m_LODcount++;
    return id;
}

LODLevel &LODGroup::GetLOD(int i)
{
    return m_LODlist[i];
}

void LODGroup::Remove(int i)
{
    auto lastid = m_LODcount;
    if (lastid != i)
    {
        m_LODlist[i] = m_LODlist[lastid];
        auto &lod = m_LODlist[lastid];
        lod.distance = 0.0f;
        lod.level = nullptr;
    }
    m_LODcount--;
}



