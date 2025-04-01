#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/geometry/batching.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/meshFilter.hpp"

MeshRender::MeshRender()
{}

void MeshRender::Init()
{
    return BaseRender::Init();
}

void MeshRender::Destroy()
{
    return BaseRender::Destroy();
}

bool MeshRender::Serialize(Stream *stream)
{
    return BaseRender::Serialize(stream);
}

bool MeshRender::Deserialize(Stream *stream)
{
    return BaseRender::Deserialize(stream);
}

void MeshRender::build()
{
    if (!HasComponent<MeshFilter>())
    {
        return;
    }

    auto meshFilter = GetComponent<MeshFilter>();

    auto meshList = meshFilter->getMesh();

    auto meshListName = AssetsManager::GetMeshListRefToName(meshList);

    auto batching = Batching::Find(meshListName);
    if (batching == nullptr)
    {
        batching = Batching::Create(meshListName);
        batching->setMesh(meshList);
    }

    batching->insert(GetGameObject()->getTransform(), &getMaterialRef());
}




