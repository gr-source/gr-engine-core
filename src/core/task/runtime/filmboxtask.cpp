#include "gr-base/core/task/runtime/filmboxtask.hpp"

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

    m_scene = filmbox.get_scene();

    load_object(m_scene.get());

    return 0;
}

#include <filesystem>
void FilmboxTask::onPostExecute(int result)
{
    if (result != 0)
    {
        return;
    }
    Console::LogInfo("Loaded: %s", m_path.c_str());

    std::filesystem::path path = m_path;

    m_filmboxManager->create_save(path.filename().c_str(), m_objectroot, std::move(m_objectlist));

    AssetsManager::RemoveTask(m_path);
}

/*
#include "gr-base/core/geometry/shader.hpp"
#include "gr-base/core/task/runtime/gameobjectsavetask.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/file/file.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/core/thread/taskmanager.hpp"
#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "imgui.h"
#include <condition_variable>
#include <mutex>
#include <stdexcept>

#include "gr-base/core/thread/threadpool.hpp"

// ********** public ********** //
FilmboxTask::FilmboxTask(const std::string &path, std::function<void ()> func) : m_name(path), bcanrender(false)
{
    add_function(func);

        Threadpool::AddTask(&FilmboxTask::doInBackground, this);
}

FilmboxTask::~FilmboxTask()
{
    invoke();
}

void FilmboxTask::add_function(std::function<void ()> func)
{
    if (func == nullptr)
    {
        return;
    }

    funclist.push_back(func);
}

void FilmboxTask::render()
{
    if (!bcanrender)
    {
        return;
    }

    if (!m_objectroot.empty())
    {
        if (ImGui::CollapsingHeader("Node"))
        {
            for (auto go : m_objectroot)
            {
                render_hierarchy(go);
            }
        }
    }

    if (ImGui::CollapsingHeader("Animation"))
    {}
}

void FilmboxTask::save()
{
    for (auto &go : m_objectroot)
    {
        TaskManager::Add<GameObjectSaveTask>(File::GetAsset("/model"), go);
    }

    // AssetsManager::RemoveTask(m_name);
}

void FilmboxTask::render_hierarchy(GameObject *go)
{
    auto transform = go->getTransform();

    if (ImGui::TreeNodeEx(go->getName().c_str(), transform->hasChildren() ? ImGuiTreeNodeFlags_None : ImGuiTreeNodeFlags_Leaf))
    {
        for (auto children : transform->getChildren())
        {
            render_hierarchy(children->getGameObject());
        }
        ImGui::TreePop();
    }
}

// ********** private ********** //
void FilmboxTask::invoke()
{
    for (auto &func : funclist)
    {
        func();
    }
}

void FilmboxTask::doInBackground()
{


    bcanrender = true;
}
*/

void FilmboxTask::load_object(const FbxScene *fbxscene)
{
    auto &root = fbxscene->getRootNode();
    
    for (auto &node : root.getChildren())
    {
        m_objectroot.push_back(create_from(node));
    }
}

GameObject *FilmboxTask::create_from(const FbxNode &node, Transform *parent)
{
    auto go = &(*m_objectlist.emplace_back(std::make_unique<GameObject>(node.getName())));
    
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
        generateMesh(&node, go);
    }

    for (auto &children : node.getChildren())
    {
        auto child = create_from(children, transform);
        if (child == nullptr)
        {
            break;
        }
    }
    return go;
}

void FilmboxTask::generateMesh(const FbxNode *node, GameObject *go)
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

        materialList.push_back(m_scene->getMaterials()[fbxMesh->getMaterialIndex()]);

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

    if (!m_bones.empty())
    {
        shaderParams.flags |= ShaderOptimize_animated;
    }

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
        auto material = renderer->createMaterial(fbxMaterial.getName(), shader);
        material->setValue("albedo", fbxMaterial.getColor());
        // material->setValue()
        
        for (auto &&[type, path] : fbxMaterial.getTextures())
        {
            switch (type) {
            case TextureType_Albedo:
                AssetsManager::LoadTexture(path, &Material::setTexture, material, 0);
                break;
            default:
                break;
            }
        }
    }
}





