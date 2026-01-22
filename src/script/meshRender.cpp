#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/gameObject.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/managers/assetsManager.hpp"
#include "gr-base/managers/batchingmanager.hpp"

#include "gr-base/managers/rendermanager.hpp"
#include "gr-base/script/meshFilter.hpp"
#include <utility>

MeshRender::MeshRender()
{}

MeshRender::~MeshRender()
{
    // if (Threadpool::IsMainThread())
        RenderManager::Remove(entityID);
    /*
    for (auto& batchID : m_batchCacheIDs)
    {
        auto& batching = BatchingManager::Get(batchID);
        batching.remove(GetGameObject()->GetID<Transform>());
    }
    */
}

void MeshRender::Init()
{
    // if (Threadpool::IsMainThread())
        MainThreadDispatcher::Add(&RenderManager::Insert, entityID);
    /*
    if (Threadpool::IsMainThread())
    {

        MainThreadDispatcher::Add([entityID = std::forward<EntityID>(entityID)]{
            auto gameobject = GameObject::Get(entityID);

            MeshFilter *meshFilter = gameobject->getComponent<MeshFilter>();
            if (meshFilter != nullptr)
            {
                auto meshes = meshFilter->getMesh();
                if (meshes.empty())
                {
                    Console::LogError("[ MeshRender ] this meshfilter don't has mesh1");
                } else
                {
                    auto meshRender = gameobject->getComponent<MeshRender>();
                    if (meshRender != nullptr)
                        meshRender->build(meshes);
                }
            } else
            {
                Console::LogError("[ MeshRender ] this gameobject don't has MeshFilter!");
            }

        });
    }
    */
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
    /*
    if (!m_batchCacheIDs.empty())
    {
        std::cout << "Need destroy: MeshRender" << std::endl;
        return;
    }

    if (m_materials.size() != meshes.size())
        return Console::LogError("[ MeshRender ] Num meshes/materials incompative.");

    size_t key = 0;

    size_t length = meshes.size();
    for (size_t i=0;i<length;i++)
    {
        MaterialID materialID = m_materials[i];
        MeshID meshID = meshes[i];

        key ^= std::hash<size_t>{}(meshID) + 0x9e3779b9 + (key << 6) + (key >> 2);
        key ^= std::hash<size_t>{}(materialID) + 0x9e3779b9 + (key << 6) + (key >> 2);

        auto batchID = BatchingManager::Find(key);
        if (batchID == InvalidBatch)
            batchID = BatchingManager::Create(key, meshID, materialID);

        Batching& batching = BatchingManager::Get(batchID);
        batching.insert(GetGameObject()->GetID<Transform>());

        m_batchCacheIDs.push_back(batchID);
    }
    */
}


