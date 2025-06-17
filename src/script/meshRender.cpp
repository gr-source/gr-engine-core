#include "gr-base/script/meshRender.hpp"

#include "gr-base/core/assetsManager.hpp"

#include "gr-base/core/geometry/batching.hpp"

#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/meshFilter.hpp"

#include "gr-base/core/file/file.hpp"
#include <cstddef>
#include <tuple>
#include <vector>

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
    if (!BaseRender::Serialize(stream))
    {
        return false;
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

    return true;
}

bool MeshRender::Deserialize(Stream *stream)
{
    if (!BaseRender::Deserialize(stream))
    {
        return false;
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
            shaderParams.flags = ShaderOptimize_dynamic;

            AssetsManager::LoadShader(shaderParams, sname, &Material::setShader, material);
            // AssetsManager::LoadShader(ShaderOptimize_DYNAMIC, sname, &Material::setShader, material);
        }
    }

    return true;
}

void MeshRender::build()
{
    if (!HasComponent<MeshFilter>())
    {
        return;
    }

    auto meshFilter = GetComponent<MeshFilter>();

    auto meshList = meshFilter->getMesh();
    if (meshList == nullptr)
    {
        return;
    }

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

        auto path = AssetsManager::GetShaderName(shader);


        ShaderParams shaderParams;
        shaderParams.name = "simple_model";
        shaderParams.flags = ShaderOptimize_dynamic;

        AssetsManager::LoadShader(shaderParams, path, &Material::setShader, material);
        
        // AssetsManager::LoadShader(ShaderOptimize_DYNAMIC, path, &Material::setShader, material);
    }

    batching->insert(GetGameObject()->getTransform(), &m_material, 1);
}




