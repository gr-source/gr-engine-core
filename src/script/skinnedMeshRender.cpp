#include "gr-base/script/skinnedMeshRender.hpp"

#include "gr-base/core/geometry/batchingmanager.hpp"
#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/batching.hpp"
#include "gr-base/core/geometry/shader.hpp"

#include "gr-base/core/thread/threadpool.hpp"
#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/ecs/gameObject.hpp"

SkinnedMeshRender::SkinnedMeshRender()
{}

void SkinnedMeshRender::onInit()
{
    BaseRender::onInit();

    if (Threadpool::IsMainThread())
        build();
}

void SkinnedMeshRender::onRelease()
{
    return BaseRender::onRelease();
}

void SkinnedMeshRender::onEnable()
{
    return BaseRender::onEnable();
}

void SkinnedMeshRender::onDisable()
{
    return BaseRender::onDisable();
}

bool SkinnedMeshRender::onSerialize(Stream *stream)
{
    if (!BaseRender::onSerialize(stream))
    {
        return false;
    }
    /*
    auto boneMapCount = m_bones.size();
    if (!stream->write(&boneMapCount, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto &bone : m_bones)
    {
        bone.serialize(stream);
    }


    auto mcount = m_material.size();

    std::vector<std::tuple<std::string, std::string>> material_shader(mcount);

    for (std::size_t i=0;i<mcount;i++)
    {
        auto shadername = AssetsManager::GetShaderName(m_material[i]->getShader());
        auto materialname = AssetsManager::GetRefMaterialToName(m_material[i]);

        material_shader[i] = std::make_tuple(materialname, shadername);
    }

    if (!stream->write(&mcount, sizeof(std::size_t)))
    {
        return false;
    }

    for (auto &&[mname, sname] : material_shader)
    {
        auto mnlen = mname.size();
        if (!stream->write(&mnlen, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(mname.c_str(), mnlen * sizeof(char)))
        {
            return false;
        }

        auto snlen = sname.size();
        if (!stream->write(&snlen, sizeof(std::size_t)))
        {
            return false;
        }

        if (!stream->write(sname.c_str(), snlen * sizeof(char)))
        {
            return false;
        }
    }
    */
    return true;
}

bool SkinnedMeshRender::onDeserialize(Stream *stream)
{
    if (!BaseRender::onDeserialize(stream))
    {
        return false;
    }
    /*
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

    std::size_t mcount = 0;
    if (!stream->read(reinterpret_cast<char *>(&mcount), sizeof(std::size_t)))
    {
        return false;
    }

    for (std::size_t i=0;i<mcount;i++)
    {
        std::size_t mnlen = 0;
        if (!stream->read(reinterpret_cast<char *>(&mnlen), sizeof(std::size_t)))
        {
            return false;
        }

        std::string mname(mnlen, '\0');
        if (!stream->read(mname.data(), mnlen * sizeof(char)))
        {
            return false;
        }

        std::size_t snlen = 0;
        if (!stream->read(reinterpret_cast<char *>(&snlen), sizeof(std::size_t)))
        {
            return false;
        }

        std::string sname(snlen, '\0');
        if (!stream->read(sname.data(), snlen * sizeof(char)))
        {
            return false;
        }

        auto material = AssetsManager::FindMaterial(mname);
        if (material != nullptr)
        {
            ShaderParams shaderParams;
            shaderParams.name = "simple_model";
            shaderParams.base = sname;
            shaderParams.flags = ShaderOptimize_dynamic | ShaderOptimize_animated;

            AssetsManager::LoadShader(shaderParams, &Material::setShader, material);
        }
    }
    */
    return true;
}

void SkinnedMeshRender::build()
{
    auto meshFilter = GetComponent<MeshFilter>();

    auto meshList = meshFilter->getMesh();

    auto meshListName = AssetsManager::GetMeshListRefToName(meshList);

    m_batching = BatchingManager::Find(meshListName);
    if (m_batching == nullptr)
        m_batching = BatchingManager::Create(meshListName, meshList);

    m_batching->insert(GetGameObject()->getTransform(), m_material);
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





