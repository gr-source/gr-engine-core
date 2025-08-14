#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/geometry/batchingmanager.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"
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
    {
        MainThreadDispatcher::Add([this]()
                {
        if (!HasComponent<MeshFilter>())
            return Console::LogError("[ SkinnedMeshRender ] this gameobject don't has MeshFilter!");

        MeshFilter *meshFilter = GetComponent<MeshFilter>();
        
        auto meshlist = meshFilter->getMesh();
        if (meshlist == nullptr)
            return Console::LogError("[ SkinnedMeshRender ] this meshfilter don't has mesh1");

        build(meshlist);
        });
    }
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

void MeshRender::build(std::vector<Mesh> *meshlist)
{
    auto meshListName = AssetsManager::GetMeshListRefToName(meshlist);

    m_batching = BatchingManager::Find(meshListName);
    if (m_batching == nullptr)
        m_batching = BatchingManager::Create(meshListName, meshlist);

    m_batching->insert(GetGameObject()->getTransform(), m_material);
}




