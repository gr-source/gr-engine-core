#include "gr-base/core/task/runtime/filmboxtask.hpp"

#include "gr-base/core/filmbox/fbxMaterial.hpp"
#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

FilmboxTask::FilmboxTask(const std::string &path, FilmboxManager *filmboxManager) : m_path(path), m_filmboxManager(filmboxManager)
{}

int FilmboxTask::doInBackground()
{
    Console::LogInfo("[ FilmboxTask ] Loading: %s", m_path.c_str());

    Filmbox filmbox;
    try
    {
        filmbox.load(m_path);
    }
    catch (const std::exception &e)
    {
        Console::LogError("[ FilmboxTask ] %s.", e.what());
        return 1;
    }

    auto scene = filmbox.get_scene();

    load_object(scene.get());

    return 0;
}

#include <filesystem>
void FilmboxTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return Console::LogError("[ FilmboxTask ] Fail to load fbx!");
    }
    Console::LogInfo("Loaded: %s", m_path.c_str());

    std::filesystem::path path = m_path;

    m_filmboxManager->create_save(path.filename().c_str(), m_objectroot);

    AssetsManager::RemoveTask(m_path);
}

void FilmboxTask::load_object(FbxScene *fbxscene)
{
    auto &root = fbxscene->getRootNode();
    
    for (auto &node : root.getChildren())
    {
        m_objectroot.push_back(create_from(fbxscene, node));
    }
}

GameObject *FilmboxTask::create_from(FbxScene *scene, const FbxNode &node, Transform *parent)
{
    auto go = GameObject::Create(node.getName());
    
    auto transform = go->getTransform();
    if (parent != nullptr)
    {
        transform->setParent(parent);
    }

    transform->setLocalPosition(node.getPosition());
    transform->setLocalRotation(node.getRotation());
    transform->setLocalScale(node.getScale());

    if (node.hasMeshe())
    {
        generateMesh(scene, &node, go);
    }

    for (auto &children : node.getChildren())
    {
        auto child = create_from(scene, children, transform);
        if (child == nullptr)
        {
            break;
        }
    }
    return go;
}

void FilmboxTask::generateMesh(FbxScene *scene, const FbxNode *node, GameObject *go)
{
    std::unordered_map<std::string, FbxBone> m_bones;
    std::vector<FbxBone> bonesList;

    std::vector<FbxMaterial> materialList;
    std::vector<Mesh> meshMap;
    std::string mesh_name;

    for (auto fbxMesh : node->getMeshes())
    {
        auto &mesh = meshMap.emplace_back(fbxMesh->getName());

        if (mesh_name.empty())
            mesh_name = fbxMesh->getName();

        materialList.push_back(scene->getMaterials()[fbxMesh->getMaterialIndex()]);

        for (auto &vertice : fbxMesh->getVertices()) {
            mesh.addVertex(vertice.position, vertice.normal, vertice.texCoord);

            if (!fbxMesh->hasBone()) {
                continue;
            }

            auto& boneWeight = mesh.addBoneWeight();

            float norm = vertice.weight[0] + vertice.weight[1] + vertice.weight[2] + vertice.weight[3];

            for (auto k=0;k<4;++k) {
                boneWeight.insert<int>(k, vertice.boneIndex[k]);
                boneWeight.insert<float>(k, norm > 0.0f ? vertice.weight[k] / norm : vertice.weight[k]);
            }
        }

        for (auto &indice : fbxMesh->getIndices()) {
            mesh.addIndice(indice);
        }

        if (fbxMesh->hasBone())
        {
            for (auto &bone : fbxMesh->getBones())
            {
                if (m_bones.find(bone.getName()) != m_bones.end())
                    continue;

                m_bones.emplace(bone.getName(), bone);
            }
        }
    }

    bonesList.resize(m_bones.size(), {"", 0});
    for (auto &&[name, bone] : m_bones)
    {
        bonesList[bone.getID()] = bone;
    }

    auto m_mesh = AssetsManager::FindMesh(mesh_name);
    if (!m_mesh) {
        m_mesh = AssetsManager::CreateMesh(mesh_name);
        for (auto &mesh : meshMap) {
            m_mesh->emplace_back(mesh);
        }
    }

    auto meshFilter = go->addComponent<MeshFilter>();
    if (!meshFilter) { return; }

    meshFilter->setMesh(m_mesh);

    auto renderer = m_bones.empty() ? static_cast<BaseRender *>(go->addComponent<MeshRender>()) : static_cast<BaseRender *>(go->addComponent<SkinnedMeshRender>());
    if (!renderer) { return; }

    if (!m_bones.empty()) {
        auto animator = go->addComponent<Animator>();
        if (!animator) { return; }

        SkinnedMeshRender *skinnedMeshRender = static_cast<SkinnedMeshRender *>(renderer);

        skinnedMeshRender->setBones(std::vector<Bone>(m_bones.size(), Bone{}));

        for (std::size_t i=0;i<bonesList.size();i++) {
            skinnedMeshRender->getBone(i)->setScale(bonesList[i].getScale());
            skinnedMeshRender->getBone(i)->setRotation(bonesList[i].getRotation());
            skinnedMeshRender->getBone(i)->setPosition(bonesList[i].getPosition());

            skinnedMeshRender->getBone(i)->setParentIndex(bonesList[i].getParentIndex());
        }
    }

    ShaderParams shaderParams;
    shaderParams.name = "simple_model";
    shaderParams.base = File::GetAsset("/shader/model/simple_model.glsl");
    shaderParams.flags = ShaderOptimize_dynamic;
    shaderParams.type = ShaderType::SHADER3D;

    auto name = GetShaderParamsName(shaderParams);

    auto shader = AssetsManager::FindShader(name);
    if (shader == nullptr)
    {
        auto info = AssetsManager::LoadShader(shaderParams,
                [&](grr::gShader *current)
                {
                    shader = current;
                });
        info->wait();
    }

    for (auto &fbxMaterial : materialList)
    {
        auto texturepath = fbxMaterial.getTextures();

        auto material = renderer->createMaterial(fbxMaterial.getName(), shader);
        material->setValue("albedo", fbxMaterial.getColor());
        material->setValue("metallic", fbxMaterial.metallic);
        material->setValue("roughness", fbxMaterial.roughness);
        material->setValue("ambientocclusion", fbxMaterial.ao);

        if (!texturepath[TextureType_Albedo].empty())
            AssetsManager::LoadTexture(texturepath[TextureType_Albedo], &Material::setTexture, material, 0);
    }
}





