#include "gr-base/core/binary/modelhandle.hpp"

#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/core/assetsManager.hpp"
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

void ModelHandle::ImportFBX(const std::string &path) {
    Filmbox::Load(path, std::bind(&ModelHandle::SetupFBX, path, std::placeholders::_1), true);
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
            task->convertToGameObject();
        }

        taskList.clear();

        taskListMap.clear();
    }

    ImGui::End();
}

void ModelHandle::SetupFBX(const std::string &path, std::shared_ptr<FbxScene> scene) {
    auto filename = std::filesystem::path(path).filename();

    auto it = taskListMap.find(filename);
    if (it != taskListMap.end()) {
        return;
    }

    taskListMap[filename] = std::make_unique<ModelHandle>(filename, scene);

    taskList.push_back(&(*taskListMap[filename]));
}

ModelHandle::ModelHandle(const std::string &name, std::shared_ptr<FbxScene> scene) : name(name), m_scene(scene) {
    auto &root = scene->getRootNode();

    for (auto &child : root.getChildren()) {
        enableNode(&child);
    }
}

void ModelHandle::render() {
    auto isOpen = ImGui::TreeNode(name.c_str());

    if (isOpen) {
        auto &rootNode = m_scene->getRootNode();
        for (auto &child : rootNode.getChildren()) {
            renderNode(&child);
        }

        ImGui::TreePop();
    }
}

void ModelHandle::renderNode(const FbxNode *node) {
    auto &children = node->getChildren();

    auto isOpen = ImGui::TreeNodeEx(node->getName().c_str(), children.empty() ? ImGuiTreeNodeFlags_Leaf : 0);

    ImGui::SameLine();
    ImGui::Checkbox("##import_checkbox_enable", &m_active[node]);
    
    if (isOpen) {
        for (auto &child : children) {
            renderNode(&child);
        }

        ImGui::TreePop();
    }
}

void ModelHandle::enableNode(const FbxNode *node) {
    m_active[node] = true;

    for (auto &child : node->getChildren()) {
        enableNode(&child);
    }
}

void ModelHandle::convertToGameObject() {
    auto &root = m_scene->getRootNode();

    std::vector<GameObject *> rootGO;

    for (auto &node : root.getChildren()) {
        auto go = CreateGameObject(&node);
        if (go != nullptr)
            rootGO.push_back(go);
    }

    for (auto go : rootGO) {
        generateScriptCache(go);
    }

    build(rootGO);
}

GameObject *ModelHandle::CreateGameObject(const FbxNode *node, Transform *parent) {
    if (!m_active[node]) { return nullptr; }

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
        generateMesh(node, go);
    }

    for (auto &child : node->getChildren()) {
        CreateGameObject(&child, transform);
    }

    return go;
}

void ModelHandle::generateMesh(const FbxNode *node, GameObject *go) {
    std::unordered_map<std::string, FbxMaterial> materialList;
    std::unordered_map<std::string, FbxBone> m_bones;
    grm::u32 verticeCount = 0;
    std::string meshName;
    Mesh m_tempMesh;

    for (auto &fbxMesh : node->getMeshes()) {
        meshName += fbxMesh.getName();

        for (auto &mIdx : fbxMesh.getMaterials()) {
            auto fbxMaterial = m_scene->getMaterial(mIdx);
            
            if (materialList.find(fbxMaterial.getName()) != materialList.end()) {
                continue;
            }
            materialList.emplace(fbxMaterial.getName(), fbxMaterial);
        }
        
        auto &vertices = fbxMesh.getVertices();

        for (auto &vertice : vertices) {
            m_tempMesh.addVertex(vertice.position, vertice.normal, vertice.texCoord, vertice.index);

            if (!fbxMesh.hasBone()) {
                continue;
            }

            auto& boneWeight = m_tempMesh.addBoneWeight();

            float norm = vertice.weight[0] + vertice.weight[1] + vertice.weight[2] + vertice.weight[3];

            for (auto k=0;k<4;++k) {
                boneWeight.insert<int>(k, vertice.boneIndex[k]);
                boneWeight.insert<float>(k, norm > 0.0f ? vertice.weight[k] / norm : vertice.weight[k]);
            }
        }

        for (auto &indice : fbxMesh.getIndices()) {
            m_tempMesh.addIndice(verticeCount + indice);
        }

        if (fbxMesh.hasBone()) {
            for (auto &bone : fbxMesh.getBones()) {
                if (m_bones.find(bone.getName()) != m_bones.end()) { continue; }

                m_bones.emplace(bone.getName(), bone);
            }
        }
        verticeCount += vertices.size();
    }

    auto m_mesh = AssetsManager::FindMesh(meshName);
    if (!m_mesh) {
        m_tempMesh.setName(meshName);

        m_mesh = AssetsManager::CreateMesh(meshName, m_tempMesh);
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

    for (auto &&[name, fbxMaterial] : materialList) {
        auto material = meshRender->addMaterial(fbxMaterial.getName(), fbxMaterial.getName(), fbxMaterial.getColor());

        m_materials[go].emplace_back(fbxMaterial.getName(), material);

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

void ModelHandle::generateScriptCache(const GameObject *go) {
    auto entity = go->getEntity();

    std::list<std::tuple<std::string, Component *>> m_list;
    for (auto [name, ptr] : entity->getComponentMap()) {
        m_list.emplace_front(name, &(*ptr));
    }

    for (auto &&[name, ptr] : m_list) {
        m_components[go].emplace_back(name, ptr);
    }

    for (auto child : go->getTransform()->getChildren()) {
        generateScriptCache(child->getGameObject());
    }
}

void ModelHandle::build(std::vector<GameObject *> gos) {
    for (auto go : gos) {
        gen(File::GetAsset("/model/" + go->getName() + ".gmdx"), go);
    }
}

void ModelHandle::gen(const std::string &path, const GameObject *go) const {
    Console::LogInfo("[ ModelHandle ] Initialize task for import: %s, %s", go->getName().c_str(), path.c_str());

    Stream stream(path, std::ios_base::out | std::ios_base::binary);

    // Header file
    {
        FileHeader header{{'g', 'r', 'm', 'x'}, 1.0f, 0};
        if (!stream.writeS(&header, 1, false)) {
            return Console::LogError("[ ModelHandle ] Fail to write header.");
        }
    }

    int code = writeData(&stream, go);
    if (code != 0) {
        Console::LogError("[ ModelHandle ] Fail to write metadata: %d", code);
    }
}

int ModelHandle::writeData(Stream *stream, const GameObject *go) const {
    // write gameobject
    {
        if (writeGameObject(stream, go) == -1) {
            return 1;
        }

        DataSegment segment = {gHeaderType_Object, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 4;
        }
        stream->flush();
    }

    // write texture
    {
        if (writeTexture(stream, go) == -1) {
            return 1;
        }
        
        DataSegment segment = {gHeaderType_Texture, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 1;
        }
        stream->flush(); 
    }

    // write material
    {
        if (writeMaterial(stream, go) == -1) {
            return 1;
        }

        DataSegment segment = {gHeaderType_Material, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 2;
        }
        stream->flush();
    }

    // write script
    {
        if (writeScript(stream, go) == -1) {
            return 1;
        }

        DataSegment segment = {gHeaderType_Script, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 3;
        }
        stream->flush();
    }

    DataSegment segment = {gHeaderType_None, 0};
    if (!stream->writeS(&segment, 1, false)) {
        return 5;
    }
    return 0;
}

int ModelHandle::writeTexture(Stream *stream, const GameObject *go) const {
    auto materials = getMaterialOfRefGameObject(go);

    auto name = go->getName();

    auto nsize = name.size();
    if (!stream->write(&nsize, sizeof(std::size_t))) {
        return 1;
    }

    if (!stream->write(name.c_str(), nsize * sizeof(char))) {
        return 2;
    }

    std::vector<grr::gTexture *> textures;
    for (auto &&[name, material] : materials) {
        auto texture0 = material->getTexture(0);
        if (texture0 == nullptr) { continue; }

        textures.push_back(texture0);
    }

    auto tcount = textures.size();

    std::cout << name << ", " << tcount << std::endl;

    if (!stream->write(&tcount, sizeof(std::size_t))) {
        return 3;
    }

    for (auto texture : textures) {
        auto path = AssetsManager::GetRefTextureName(texture);

        auto psize = path.size();
        if (!stream->write(&psize, sizeof(std::size_t))) {
            return 4;
        }

        if (!stream->write(path.c_str(), psize * sizeof(char))) {
            return 5;
        }
    }

    auto children = go->getTransform()->getChildren();
    auto ccount = children.size();

    if (!stream->write(&ccount, sizeof(std::size_t))) {
        return 6;
    }

    int result = 0;
    for (auto child : children) {
        result = writeMaterial(stream, child->getGameObject());
        if (result != 0) {
            break;
        }
    }

    return 7 + result;
}

int ModelHandle::writeMaterial(Stream *stream, const GameObject *go) const {
    auto materials = getMaterialOfRefGameObject(go);

    auto name = go->getName();

    auto nsize = name.size();
    if (!stream->write(&nsize, sizeof(std::size_t))) {
        return 1;
    }

    if (!stream->write(name.c_str(), nsize * sizeof(char))) {
        return 2;
    }

    auto size = materials.size();
    if (!stream->write(&size, sizeof(std::size_t))) {
        return 3;
    }

    for (auto &&[name, material] : materials) {
        size = name.size();

        if (!stream->write(&size, sizeof(std::size_t))) {
            return 4;
        }

        if (!stream->write(name.data(), size * sizeof(char))) {
            return 5;
        }

        std::size_t bufferSize = 0;
        {
            Stream temp;
            material->serialize(&temp);
            bufferSize = temp.getDataSize();

            temp.close();
        }

        if (!stream->write(&bufferSize, sizeof(std::size_t))) {
            return 6;
        }

        material->serialize(stream);
    }

    auto children = go->getTransform()->getChildren();
    size = children.size();

    if (!stream->write(&size, sizeof(std::size_t))) {
        return 6;
    }

    int result = 0;
    for (auto child : children) {
        result = writeMaterial(stream, child->getGameObject());
        if (result != 0) {
            break;
        }
    }

    return 7 + result;
}

int ModelHandle::writeScript(Stream *stream, const GameObject *go) const {
    auto componentList = getScriptOfRefGameObject(go);

    auto goname = go->getName();
    auto sname = goname.size();

    if (!stream->write(&sname, sizeof(std::size_t))) {
        return 1;
    }

    if (!stream->write(goname.c_str(), sname * sizeof(char))) {
        return 2;
    }

    auto ccount = componentList.size();
    if (!stream->write(&ccount, sizeof(std::size_t))) {
        return 3;
    }

    for (auto &&[name, component] : componentList) {
        // Component Name
        sname = name.size();

        if (!stream->write(&sname, sizeof(std::size_t))) {
            return 4;
        }

        if (!stream->write(name.data(), sname * sizeof(char))) {
            return 5;
        }

        Stream temp;
        component->Serialize(&temp);
        std::size_t data_size = temp.getDataSize();

        if (!stream->write(&data_size, sizeof(std::size_t))) {
            return 6;
        }
        component->Serialize(stream);

        temp.close();
    }

    auto children = go->getTransform()->getChildren();
    auto csize = children.size();

    if (!stream->write(&csize, sizeof(std::size_t))) {
        return 7;
    }

    int result = 0;
    for (auto child : children) {
        result = writeScript(stream, child->getGameObject());
        if (result != 0) {
            break;
        }
    }

    return result != 0 ? 8 + result : 0;
}

int ModelHandle::writeGameObject(Stream *stream, const GameObject *go) const {
    auto name = go->getName();
    auto nSize = name.size();

    auto tag = go->getTag();
    auto tSize = tag.size();

    stream->write(&nSize, sizeof(std::size_t));
    stream->write(name.data(), nSize * sizeof(char));

    stream->write(&tSize, sizeof(std::size_t));
    stream->write(tag.data(),  tSize * sizeof(char));

    auto active = go->getActive();
    auto enable = go->getEnable();
    stream->write(&active,  sizeof(bool));
    stream->write(&enable,  sizeof(bool));

    auto transform = go->getTransform();
    auto children = transform->getChildren();

    auto cSize = children.size();
    stream->write(&cSize, sizeof(std::size_t));

    for (auto &child : children) {
        auto object = child->getGameObject();

        writeGameObject(stream, object);
    }
    return 0;
}

std::vector<std::tuple<std::string, Component *>> ModelHandle::getScriptOfRefGameObject(const GameObject *go) const {
    auto it = m_components.find(go);
    if (it == m_components.end()) {
        return std::vector<std::tuple<std::string, Component *>>();
    }
    return it->second;
}

std::vector<std::tuple<std::string, Material *>> ModelHandle::getMaterialOfRefGameObject(const GameObject *go) const {
    auto it = m_materials.find(go);
    if (it == m_materials.end()) {
        return std::vector<std::tuple<std::string, Material *>>();
    }
    return it->second;
}

