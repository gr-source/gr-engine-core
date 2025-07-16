#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/geometry/batchingmanager.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/geometry/shader.hpp"

#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/meshFilter.hpp"

MeshRender::MeshRender()
{
}

void MeshRender::onInit()
{
    BaseRender::onInit();

    if (Threadpool::IsMainThread())
        build();
}

void MeshRender::onRelease()
{
    return BaseRender::onRelease();
}

bool MeshRender::onSerialize(Stream *stream)
{
    if (!BaseRender::onSerialize(stream))
    {
        return false;
    }
    return true;
}

bool MeshRender::onDeserialize(Stream *stream)
{
    if (!BaseRender::onDeserialize(stream))
    {
        return false;
    }
    return true;
}

void MeshRender::build()
{
    if (!HasComponent<MeshFilter>())
    {
        return;
    }

    auto meshFilter = GetComponent<MeshFilter>();

    auto meshList = meshFilter->getMesh();
    if (meshList == nullptr)
    {
        return;
    }

    auto meshListName = AssetsManager::GetMeshListRefToName(meshList);

    m_batching = BatchingManager::Find(meshListName);
    if (m_batching == nullptr)
        m_batching = BatchingManager::Create(meshListName, meshList);

    m_batching->insert(GetGameObject()->getTransform(), m_material);
}




