#include "gr-base/core/binary/modelhandle.hpp"

#include "gr-base/core/animation/keyframe.hpp"
#include "gr-base/core/filmbox/fbxScene.hpp"
#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/window.hpp"

#include "gr-base/script/skinnedMeshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include "gr-base/core/animation/bone.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/task/animationsavetask.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <cstddef>
#include <imgui.h>

#include <filesystem>
#include <mutex>
#include <vector>

std::unordered_map<std::string, std::unique_ptr<ModelHandle>> ModelHandle::taskListMap;

std::vector<ModelHandle *> ModelHandle::taskList;

std::mutex ModelHandle::mutex;

std::atomic<bool> ModelHandle::bHasTasks = false;

void ModelHandle::ImportFBX(const std::string &path) {
    std::function<void (std::unique_ptr<FbxScene>)> func = std::bind(&ModelHandle::SetupFBX, path, std::placeholders::_1);
    if (func)
        Threadpool::AddTask(&Filmbox::Load, path, func, true);
}

void ModelHandle::Render()
{
    if (!bHasTasks)
    {
        return;
    }

    auto viewportsize = Window::GetWindowSize();

    ImVec2 windowSize(300, 400);

    ImGui::SetNextWindowPos(ImVec2{(static_cast<float>(viewportsize.x) / 2) - (windowSize.x / 2), (static_cast<float>(viewportsize.y) / 2) - (windowSize.y / 2)});
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("Import Menu", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    auto winsize = ImGui::GetContentRegionAvail();

    {

        std::unique_lock<std::mutex> lock(mutex);
        for (auto task : taskList)
        {
            lock.unlock();

            if (task->isCanRender)
                task->render();

            lock.lock();
        }
    }

    auto spacing = ImGui::GetStyle().ItemSpacing;

    ImGui::SetCursorPosY(winsize.y - spacing.y);
    if (ImGui::Button("Cancel", ImVec2{winsize.x / 2.0f, 0.0f})) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            taskList.clear();

            taskListMap.clear();
        }

        bHasTasks = false;
    }

    ImGui::SetCursorPos(ImVec2{(winsize.x / 2.0f) + spacing.x, winsize.y - spacing.y});
    if (ImGui::Button("Save", ImVec2{winsize.x / 2.0f, 0.0f})) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            for (auto task : taskList)
            {
                lock.unlock();

                task->run();

                lock.lock();
            }

            taskList.clear();

            taskListMap.clear();
        }
        bHasTasks = false;
    }

    ImGui::End();
}

void ModelHandle::SetupFBX(const std::string &path, std::unique_ptr<FbxScene> scene)
{
    auto filename = std::filesystem::path(path).filename();

    {
        std::lock_guard<std::mutex> lock(mutex);

        auto it = taskListMap.find(filename);
        if (it != taskListMap.end())
        {
            return;
        }
    }

    auto modelHandle = std::make_unique<ModelHandle>(filename, std::move(scene));

    {
        std::lock_guard<std::mutex> lock(mutex);
        taskListMap.emplace(filename, std::move(modelHandle));
        
        taskList.push_back(&(*taskListMap[filename]));
    }

    bHasTasks = true;
}

ModelHandle::ModelHandle(const std::string &name, std::unique_ptr<FbxScene> scene) : name(name), isCanRender(true) {
    convertToGameObject(scene.get());

    generateAnimation(scene.get());
}

#include "gr-base/core/task/saveGameObjectTask.hpp"
void ModelHandle::run()
{
    saveNode();

    saveAnimation();
}

void ModelHandle::render()
{
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

    auto isOpen = ImGui::TreeNode(name.c_str());
    if (!isOpen)
    {
        return;
    }

    if (!rootNode.empty())
    {
        if (ImGui::TreeNode("Node"))
        {
            for (auto go : rootNode)
            {
                renderNode(go);
            }

            ImGui::TreePop();
        }
    }

    if (!mAnimationLits.empty())
    {
        if (ImGui::TreeNode("Animation"))
        {
            
            for (auto &animation : mAnimationLits)
            {
                renderAnimation(&(*animation));
            }
            ImGui::TreePop();
        }
    }

    ImGui::TreePop();
}

void ModelHandle::renderNode(const GameObject *go)
{
    auto transform = go->getTransform();

    auto children = transform->getChildren();

    ImGui::PushID(transform);

    auto isOpen = ImGui::TreeNodeEx(go->getName().c_str(), children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);

    ImGui::SameLine();
    ImGui::Checkbox("##import_checkbox_enable", &mMarkToSaveObject[go]);
    
    if (!isOpen)
    {
        ImGui::PopID();
        return;
    }

    for (auto child : children)
    {
        renderNode(child->getGameObject());
    }

    ImGui::TreePop();

    ImGui::PopID();
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
    mMarkToSaveObject[go] = true;

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

void ModelHandle::generateMesh(const FbxScene *scene, const FbxNode *node, GameObject *go)
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

        if (fbxMesh->hasBone()) {
            for (auto &bone : fbxMesh->getBones()) {
                if (m_bones.find(bone.getName()) != m_bones.end()) { continue; }

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

    for (auto &fbxMaterial : materialList) {
        auto material = renderer->createMaterial(fbxMaterial.getName());

        if (material->getShader() == nullptr)
        {
            AssetsManager::LoadShader(File::GetAsset("/shader/model/simple_model.glsl"), &Material::setShader, material);
        }

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

void ModelHandle::generateAnimation(FbxScene *fbxScene)
{
    for (auto &fbxAnimation : fbxScene->getAnimations())
    {
        auto &channels = fbxAnimation.getChannels();

        auto animation = &(*mAnimationLits.emplace_back(std::make_unique<Animation>()));
        animation->setDuration(fbxAnimation.getDuration());
        animation->setName(fbxAnimation.getName());
        animation->setTick(fbxAnimation.getTick());
        animation->setKeyFrame(std::vector<Keyframe>(channels.size(), Keyframe{}));

        for (std::size_t i=0;i<channels.size();i++)
        {
            auto &channel = channels[i];

            auto keyframe = animation->getKeyFrame(i);

            for (auto &key : channel.getKeysScale())
            {
                keyframe->addKeyScale(key.time, key.value);
            }

            for (auto &key : channel.getKeysRotation())
            {
                keyframe->addKeyRotation(key.time, key.value);
            }

            for (auto &key : channel.getKeysPosition())
            {
                keyframe->addKeyPosition(key.time, key.value);
            }
        }

        mMarkToSaveAnimation.emplace(animation, true);
    }
}

void ModelHandle::renderAnimation(Animation *animation)
{
    auto name = animation->getName();

    ImGui::Checkbox(name.c_str(), &mMarkToSaveAnimation[animation]);
}

void ModelHandle::saveNode()
{
    removeObjectMarkUnsave();

    for (auto go : rootNode)
    {
        SaveGameObjectTask SaveGameObjectTask(go, File::GetAsset("/model/" + go->getName() + ".gmdx"));
    }
}

void ModelHandle::removeObjectMarkUnsave()
{
    for (auto it=rootNode.begin();it!=rootNode.end();)
    {
        if (!mMarkToSaveObject[*it])
        {
            it = rootNode.erase(it);
            continue;
        }
        for_each((*it)->getTransform());
        it++;
    }
}

void ModelHandle::for_each(Transform *transform)
{

    for (auto child : transform->getChildren())
    {
        if (!mMarkToSaveObject[child->getGameObject()])
        {
            child->setParent(nullptr);
            continue;
        }
        for_each(child);
    }
}

void ModelHandle::saveAnimation()
{
    for (auto &animation : mAnimationLits)
    {
        if (!mMarkToSaveAnimation[animation.get()])
        {
            continue;
        }
        AnimationSaveTask animationsavetask(File::GetAsset("/animation"), &(*animation));
    }
}


