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
{}

void MeshRender::Init()
{
    BaseRender::Init();

    if (Threadpool::IsMainThread())
        build();
}

void MeshRender::Destroy()
{
    return BaseRender::Destroy();
}

bool MeshRender::Serialize(Stream *stream)
{
    if (!BaseRender::Serialize(stream))
    {
        return false;
    }
    return true;
}

bool MeshRender::Deserialize(Stream *stream)
{
    if (!BaseRender::Deserialize(stream))
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




