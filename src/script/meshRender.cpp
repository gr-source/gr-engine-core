#include "gr-base/script/meshRender.hpp"

#include "gr-base/broadphase/bvhtree.hpp"
#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/geometry/batchingmanager.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"

MeshRender::MeshRender() : m_batchID(InvalidBatch)
{}

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
        
            auto meshes = meshFilter->getMesh();
            if (meshes.empty())
                return Console::LogError("[ SkinnedMeshRender ] this meshfilter don't has mesh1");

            Boundbox boundbox;
            for (auto &meshID : meshes)
            {
                auto &mesh = AssetsManager::GetMesh(meshID);

                boundbox = boundbox.Encapsulate(mesh.getBoundbox());
            }

            auto transform = getGameObject()->getTransform();

            auto &model = transform->getGlobalMatrix();
            auto min = model * Vector4{boundbox.min.x, boundbox.min.y, boundbox.min.z, 1.0f};
            auto max = model * Vector4{boundbox.max.x, boundbox.max.y, boundbox.max.z, 1.0f};

            bvh::Tree::Insert(bvh::Bounds{{min.x, min.y, min.z}, {max.x, max.y, max.z}}, this);

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

    m_batchID = BatchingManager::Find(batchname);
    if (m_batchID == InvalidBatch)
        m_batchID = BatchingManager::Create(batchname, meshes);

    auto &batch = BatchingManager::Get(m_batchID);

    batch.insert(GetGameObject()->getTransform(), m_materials);
}




