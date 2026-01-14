#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/batchingmanager.hpp"

#include "gr-base/script/meshFilter.hpp"


MeshRender::MeshRender() : m_batchID(InvalidBatch)
{
    // MainThreadDispatcher::Add([this]()

    if (Threadpool::IsMainThread())
    {
        MeshFilter *meshFilter = entity->getComponent<MeshFilter>();
        if (meshFilter != nullptr)
        {
            auto meshes = meshFilter->getMesh();
            if (meshes.empty())
            {
                Console::LogError("[ SkinnedMeshRender ] this meshfilter don't has mesh1");
            } else
            {
                MainThreadDispatcher::Add(&MeshRender::build, this, meshes);
            }
        } else
        {
            Console::LogError("[ SkinnedMeshRender ] this gameobject don't has MeshFilter!");
        }
    }
}

MeshRender::~MeshRender()
{
    if (m_batchID != InvalidBatch)
        BatchingManager::Destroy(m_batchID);
}

bool MeshRender::Serialize(Stream* stream) const
{
    return BaseRender::Serialize(stream);
}

bool MeshRender::Deserialize(Stream* stream)
{
    return BaseRender::Deserialize(stream);
}

void MeshRender::build(const std::vector<MeshID>& meshes)
{
    std::string batchname;
    for (auto &meshID : meshes)
        batchname += AssetsManager::GetMeshName(meshID);

    if (m_batchID != InvalidBatch)
        BatchingManager::Destroy(m_batchID);

    m_batchID = BatchingManager::Find(batchname);
    if (m_batchID == InvalidBatch)
        m_batchID = BatchingManager::Create(batchname, meshes);

    auto &batch = BatchingManager::Get(m_batchID);

    batch.insert(GetGameObject()->getTransform(), m_materials);
}


