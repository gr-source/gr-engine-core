#include "gr-base/script/skinnedMeshRender.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/core/geometry/batching.hpp"

SkinnedMeshRender::SkinnedMeshRender()
{}

void SkinnedMeshRender::Init()
{
    return BaseRender::Init();
}

void SkinnedMeshRender::Destroy()
{
    return BaseRender::Destroy();
}

void SkinnedMeshRender::Enable()
{
    return BaseRender::Enable();
}

void SkinnedMeshRender::Disable()
{
    return BaseRender::Disable();
}

bool SkinnedMeshRender::Serialize(Stream *stream)
{
    auto boneMapCount = m_bones.size();
    if (!stream->write(&boneMapCount, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto &bone : m_bones)
    {
        bone.serialize(stream);
    }

    return BaseRender::Serialize(stream);
}

bool SkinnedMeshRender::Deserialize(Stream *stream)
{
    std::size_t boneMapCount = 0;
    if (!stream->read(reinterpret_cast<char *>(&boneMapCount), sizeof(std::size_t)))
    {
        return false;
    }

    m_bones.resize(boneMapCount);

    for (std::size_t i=0;i<boneMapCount;i++)
    {
        m_bones[i].deserialize(stream);
    }

    return BaseRender::Deserialize(stream);
}

void SkinnedMeshRender::build()
{
    auto meshFilter = GetComponent<MeshFilter>();

    auto meshList = meshFilter->getMesh();

    auto meshListName = AssetsManager::GetMeshListRefToName(meshList);

    auto batching = Batching::Find(meshListName);
    if (batching == nullptr)
    {
        batching = Batching::Create(meshListName);
        batching->setMesh(meshList);
    }

    for (auto &material : m_material)
    {
        auto shader = material->getShader();
        if (shader == nullptr)
        {
            continue;
        }
        auto path = AssetsManager::GetShaderRefToName(shader);
        if (!File::Exists(path))
        {
            continue;
        }
        AssetsManager::LoadShader(path + ";define={USE_MODEL,ENABLE_ANIMATION}", &Material::setShader, material);
    }

    batching->insert(GetGameObject()->getTransform(), &m_material);
}

void SkinnedMeshRender::setBones(const std::vector<Bone> &bones)
{
    m_bones = bones;
}

Bone *SkinnedMeshRender::getBone(int index)
{
    return &m_bones[index];
}

std::size_t SkinnedMeshRender::getBoneCount() const
{
    return m_bones.size();
}





