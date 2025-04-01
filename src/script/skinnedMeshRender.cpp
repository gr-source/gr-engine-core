#include "gr-base/script/skinnedMeshRender.hpp"

#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"

#include "gr-base/ecs/gameObject.hpp"

SkinnedMeshRender::SkinnedMeshRender()
{}

void SkinnedMeshRender::Init()
{
    BaseRender::Init();
}

void SkinnedMeshRender::Destroy()
{
    BaseRender::Destroy();
}

void SkinnedMeshRender::Enable()
{
    BaseRender::Enable();
}

void SkinnedMeshRender::Disable()
{
    BaseRender::Disable();
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
    std::cout << "Build SkinnedMeshRender: " << GetGameObject()->getName() << std::endl;
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





