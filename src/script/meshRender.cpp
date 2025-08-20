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

MeshRender::MeshRender() : batchingdynamic(nullptr)
{}

void MeshRender::onInit()
{
    BaseRender::onInit();

    batchingdynamic = nullptr;

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

void MeshRender::build()
{
    /*
    if (!HasComponent<MeshFilter>())
        return Console::LogError("[ SkinnedMeshRender ] this gameobject don't has MeshFilter!");

    MeshFilter *meshFilter = GetComponent<MeshFilter>();
        
    auto meshlist = meshFilter->getMesh();
    if (meshlist == nullptr)
        return Console::LogError("[ SkinnedMeshRender ] this meshfilter don't has mesh1");

    build(meshlist);
    */
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

    auto m_batchID = BatchingManager::Find(meshListName);
    if (m_batchID == InvalidBatch)
        m_batchID = BatchingManager::Create(meshListName, meshlist);

    Batching *batching = BatchingManager::Get(m_batchID);
    batchingdynamic = batching->insert(GetGameObject()->getTransform(), m_material);
}




