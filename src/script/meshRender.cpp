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

MeshRender::MeshRender() : batchingdynamic(nullptr), m_batchID(InvalidBatch)
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
        
            auto meshes = meshFilter->getMesh();
            if (meshes.empty())
                return Console::LogError("[ SkinnedMeshRender ] this meshfilter don't has mesh1");

            build(meshes);
        });
    }
}

void MeshRender::onRelease()
{
    return BaseRender::onRelease();
}

bool MeshRender::onSerialize(Stream *stream)
{
    return BaseRender::onSerialize(stream);
}

bool MeshRender::onDeserialize(Stream *stream)
{
    return BaseRender::onDeserialize(stream);
}

void MeshRender::build(const std::vector<MeshID> &meshes)
{
    std::string batchname;
    for (auto &meshID : meshes)
        batchname += AssetsManager::GetMeshName(meshID);

    BatchID batchID = BatchingManager::Find(batchname);
    if (batchID == InvalidBatch)
        batchID = BatchingManager::Create(batchname, meshes);

    auto &batch = BatchingManager::Get(batchID);

    batch.insert(GetGameObject()->getTransform(), m_materials);
}




