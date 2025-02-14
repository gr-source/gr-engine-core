#include "gr-base/core/binary/modelhandle.hpp"

#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/window.hpp"

#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"

#include <imgui.h>

#include <functional>
#include <filesystem>

std::unordered_map<std::string, std::unique_ptr<ModelHandle>> ModelHandle::taskListMap;

std::vector<ModelHandle *> ModelHandle::taskList;

std::mutex ModelHandle::mutex;

void ModelHandle::ImportFBX(const std::string &path) {
    std::function<void (std::unique_ptr<FbxScene>)> func = std::bind(&ModelHandle::SetupFBX, path, std::placeholders::_1);
    if (func)
        Threadpool::AddTask(&Filmbox::Load, path, func, true);
}

void ModelHandle::Render() {
    if (taskList.empty()) { return; }

    auto viewportsize = Window::GetWindowSize();

    ImVec2 windowSize(300, 400);

    ImGui::SetNextWindowPos(ImVec2{(viewportsize.x / 2) - (windowSize.x / 2), (viewportsize.y / 2) - (windowSize.y / 2)});
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("Import Menu", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    auto winsize = ImGui::GetContentRegionAvail();

    for (auto task : taskList) {
        if (task->isCanRender)
            task->render();
    }

    auto spacing = ImGui::GetStyle().ItemSpacing;

    ImGui::SetCursorPosY(winsize.y - spacing.y);
    if (ImGui::Button("Cancel", ImVec2{winsize.x / 2.0f, 0.0f})) {
        taskList.clear();

        taskListMap.clear();
    }

    ImGui::SetCursorPos(ImVec2{(winsize.x / 2.0f) + spacing.x, winsize.y - spacing.y});
    if (ImGui::Button("Save", ImVec2{winsize.x / 2.0f, 0.0f})) {
        for (auto task : taskList) {
            task->run();
        }

        taskList.clear();

        taskListMap.clear();
    }

    ImGui::End();
}

void ModelHandle::SetupFBX(const std::string &path, std::unique_ptr<FbxScene> scene) {
    auto filename = std::filesystem::path(path).filename();

    auto it = taskListMap.find(filename);
    if (it != taskListMap.end()) {
        return;
    }

    taskListMap[filename] = std::make_unique<ModelHandle>(filename, std::move(scene));

    taskList.push_back(&(*taskListMap[filename]));
}

ModelHandle::ModelHandle(const std::string &name, std::unique_ptr<FbxScene> scene) : name(name), isCanRender(true) {
    auto &root = scene->getRootNode();

    convertToGameObject(scene.get());
}

#include "gr-base/core/task/saveGameObjectTask.hpp"
void ModelHandle::run() {
    for (auto go : rootNode) {
        SaveGameObjectTask SaveGameObjectTask(go, File::GetAsset("/model/" + go->getName() + ".gmdx"));
    }
}

void ModelHandle::render() {
    auto isOpen = ImGui::TreeNode(name.c_str());
    if (!isOpen) { return; }

    for (auto go : rootNode) {
        renderNode(go);
    }

    ImGui::TreePop();
}

void ModelHandle::renderNode(const GameObject *go) {
    auto transform = go->getTransform();

    auto children = transform->getChildren();

    auto isOpen = ImGui::TreeNodeEx(go->getName().c_str(), children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);

    ImGui::SameLine();
    ImGui::Checkbox("##import_checkbox_enable", &mModelCanImport[go]);
    
    if (!isOpen) { return; }

    for (auto child : children) {
        renderNode(child->getGameObject());
    }

    ImGui::TreePop();
}

void ModelHandle::convertToGameObject(FbxScene *scene) {
    auto &root = scene->getRootNode();

    for (auto &node : root.getChildren()) {
        auto go = CreateGameObject(scene, &node);
        if (go != nullptr)
            rootNode.push_back(go);
    }

    for (auto root : rootNode) {
        enableNode(root);
    }
}

void ModelHandle::enableNode(const GameObject *go) {
    mModelCanImport[go] = true;

    auto transform = go->getTransform();
    for (auto child : transform->getChildren()) {
        enableNode(child->getGameObject());
    }
}

GameObject *ModelHandle::CreateGameObject(const FbxScene *scene, const FbxNode *node, Transform *parent) {
    gameObjects.push_back(std::make_unique<GameObject>(node->getName()));

    auto go = &(*gameObjects.back());

    auto transform = go->addComponent<Transform>();
    go->setTransform(transform);

    transform->setLocalPosition(node->getPosition());
    transform->setLocalRotation(node->getRotation());
    transform->setLocalScale(node->getScale());
    if (parent != nullptr) {
        transform->setParent(parent);
    }

    if (node->hasMeshe()) {
        generateMesh(scene, node, go);
    }

    for (auto &child : node->getChildren()) {
        CreateGameObject(scene, &child, transform);
    }

    return go;
}

void ModelHandle::generateMesh(const FbxScene *scene, const FbxNode *node, GameObject *go) {
    std::unordered_map<std::string, FbxBone> m_bones;
    std::vector<FbxMaterial> materialList;
    std::vector<Mesh> meshMap;
    std::string mesh_name;

    for (auto fbxMesh : node->getMeshes()) {
        auto &mesh = meshMap.emplace_back(fbxMesh->getName());

        if (mesh_name.empty())
            mesh_name = fbxMesh->getName();

        materialList.push_back(scene->getMaterials()[fbxMesh->getMaterialIndex()]);

        for (auto &vertice : fbxMesh->getVertices()) {
            mesh.addVertex(vertice.position, vertice.normal, vertice.texCoord, materialList.size() - 1);

            if (!fbxMesh->hasBone()) {
                continue;
            }

            /*
            auto& boneWeight = m_tempMesh.addBoneWeight();

            float norm = vertice.weight[0] + vertice.weight[1] + vertice.weight[2] + vertice.weight[3];

            for (auto k=0;k<4;++k) {
                boneWeight.insert<int>(k, vertice.boneIndex[k]);
                boneWeight.insert<float>(k, norm > 0.0f ? vertice.weight[k] / norm : vertice.weight[k]);
            }
            */
        }

        for (auto &indice : fbxMesh->getIndices()) {
            mesh.addIndice(indice);
        }

        /*
        if (fbxMesh.hasBone()) {
            for (auto &bone : fbxMesh.getBones()) {
                if (m_bones.find(bone.getName()) != m_bones.end()) { continue; }

                m_bones.emplace(bone.getName(), bone);
            }
        }
        verticeCount += vertices.size();
        */
    }
    // */

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

    auto meshRender = go->addComponent<MeshRender>();
    if (!meshRender) { return; }

    if (!m_bones.empty()) {
        auto animator = go->addComponent<Animator>();
        if (!animator) { return; }

        for (auto &&[name, bone] : m_bones) {
            animator->m_bones[name] = std::make_shared<Bone>(name, bone.getPosition(), bone.getScale(), bone.getRotation(), bone.getID());
        }
    }

    for (auto &fbxMaterial : materialList) {
        auto material = meshRender->addMaterial(fbxMaterial.getName(), fbxMaterial.getColor());

        for (auto &&[type, path] : fbxMaterial.getTextures()) {
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


