#include "gr-base/core/binary/model3DHandler.hpp"

#include "gr-base/core/binary/gHeader.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/threadpool.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/core/file/stream.hpp"
#include "gr-base/core/file/file.hpp"

#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include "gr-base/script/meshRender.hpp"
#include "gr-base/script/meshFilter.hpp"
#include "gr-base/script/transform.hpp"
#include "gr-base/script/animator.hpp"

#include "gr-base/core/animation/bone.hpp"

std::mutex Model3DHandler::m_mutex;

void Model3DHandler::ImportFBX(const std::string &path) {
    Threadpool::AddTask(&Filmbox::Load, path, &Model3DHandler::SetupScene, true);
}

void Model3DHandler::ExportGRMX(const std::string &path, const GameObject *gameObject) {
    auto name = gameObject->getName();

    Console::LogInfo("[ Model3DHandler ] Writing GameObject: %s", name.c_str());

    Stream stream(path, std::ios_base::out | std::ios_base::binary);

    // Header file
    {
        FileHeader header{{'g', 'r', 'm', 'x'}, 1.0f, 0};
        stream.writeS(&header, 1, false);
    }

    // data
    auto result = WriteData(&stream, gameObject);
    if (result != 0) {
        return Console::LogError("[ Model3DHandler ] Falied to write data: %d", result);
    }
    Console::LogInfo("[ Model3DHandler ] Write GameObject(%s) sucessfull.", name.c_str());
}

void Model3DHandler::ImportGRMX(const std::string &path) {
    return Threadpool::AddTask(&Model3DHandler::LoadGRMX, path);
}

void Model3DHandler::LoadGRMX(const std::string &path) {
    Stream stream(path, std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        return Console::LogError("[ Model3DHandler ] Falied to open file: %s", path.c_str());
    }

    FileHeader header;
    if (!stream.readS(&header, 1)) {
        return Console::LogError("[ Model3DHandler ] Falied to read header binary.");
    }
    
    if (header.signature[0] != 'g' && header.signature[1] != 'r' && header.signature[2] != 'm' && header.signature[3] != 'x') {
        return Console::LogError("[ Model3DHandler ] Invalid binary type.");
    }

    Console::LogInfo("[ Model3DHandler ] GRMX loading model.");

    ReadData(&stream);

    Console::LogInfo("[ Model3DHandler ] Scene loaded sucess full.");
}

void InstantiateObject(GameObject *gameObject) {
    if (!gameObject) {
        return Console::LogError("[ Model3DHandler ] GameObject invalid pointer.");
    }
    Scene::GetCurrentScene()->insert(gameObject);

    Scene::Init(gameObject);
}

int Model3DHandler::ReadData(Stream *stream) {
    GameObject *gameObject = nullptr;
    std::unordered_map<std::string, GameObject *> m_gameObjectTemp;

    while (true) {
        DataSegment segment;
        if (!stream->readS(&segment, 1)) {
            break;
        }

        if (segment.type == gHeaderType_None && segment.sizeData == 0) {
            break;
        }

        auto next_position = stream->tellg() + static_cast<std::streampos>(segment.sizeData);

        auto result = -1;
        switch (segment.type) {
        case gHeaderType_Object:
            result = ReadGameObject(stream, nullptr, &gameObject, m_gameObjectTemp);
            break;
        case gHeaderType_Script:
            result = ReadScript(stream, m_gameObjectTemp);
            break;
        default:
            break;
        }

        if (result != 0) {
            Console::LogError("[ Model3DHandler ] Falied to load segment: %s code: %d", struct_name[segment.type], result);
        }

        if (stream->tellg() != next_position)
            stream->seekg(next_position, std::ios_base::beg);
    }

    Event::Add(&InstantiateObject, gameObject);

    return 0;
}

void CreateGameObject(std::string name, std::string tag, bool enable, bool active, Transform *parent, Transform *&out, std::condition_variable &cv, std::mutex &mutex, GameObject **gameOut) {
    auto gameObject = GameObject::Create(name, tag);
    if (!gameObject) {
        return;
    }
    if (!(*gameOut)) {
        *gameOut = gameObject;
    }

    gameObject->setEnable(active);
    gameObject->setActive(active);

    auto transform = gameObject->getTransform();
    if (parent) {
        transform->setParent(parent);
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        out = transform;
    }
    cv.notify_one();
}

int Model3DHandler::ReadGameObject(Stream *stream, Transform *parent, GameObject **out, std::unordered_map<std::string, GameObject *> &tempList) {
    std::size_t nsize = 0;

    // write-name
    if (!stream->readS(&nsize, 1)) {
        return 1;
    }

    std::string name(nsize, '\0');
    if (!stream->readS(name.data(), nsize)) {
        return 2;
    }

    // write-tag
    if (!stream->readS(&nsize, 1)) {
        return 3;
    }

    std::string tag(nsize, '\0');
    if (!stream->readS(tag.data(), nsize)) {
        return 4;
    }

    auto bActive = false;
    auto bEnable = false;

    // write-status
    {
        if (!stream->readS(&bActive, 1)) {
            return 5;
        }

        if (!stream->readS(&bEnable, 1)) {
            return 6;
        }
    }

    Transform *transform = nullptr;
    std::condition_variable cv;

    Event::Add(&CreateGameObject, name, tag, bEnable, bActive, parent, std::ref(transform), std::ref(cv), std::ref(m_mutex), out);

    std::unique_lock<std::mutex> lock(m_mutex);
    cv.wait(lock, [&transform] {
        return transform != nullptr;
    });
    tempList.emplace(name, transform->getGameObject());
    lock.unlock();

    std::size_t children_size = 0;
    if (!stream->readS(&children_size, 1)) {
        return 7;
    }
    
    int result = 0;
    for (int i=0;i<children_size;i++) {
        result = ReadGameObject(stream, transform, out, tempList);
        if (result != 0) {
            break;
        }
    }
    transform = nullptr;

    return result != 0 ? (7 + result) : 0;
}

void SetupScript(GameObject *gameObject, std::string path, std::streampos position, std::string componentName) {
    auto entt = gameObject->getEntity();

    auto component = entt->hasComponent(componentName) ? entt->getComponent(componentName) : entt->addComponent(componentName);
    if (!component) {
        return;
    }

    Stream stream(path, std::ios_base::in | std::ios_base::out);
    stream.seekg(position, std::ios_base::beg);

    component->Deserialize(&stream);
}

int Model3DHandler::ReadScript(Stream *stream, std::unordered_map<std::string, GameObject *> &tempList) {
    std::size_t size = 0;

    if (!stream->readS(&size, 1)) {
        return 1;
    }

    for (std::size_t i=0;i<size;i++) {
        std::size_t nsize = 0;

        // GameObject - name
        if (!stream->readS(&nsize, 1)) {
            return 2;
        }
        
        std::string objectName(nsize, '\0');
        if (!stream->readS(objectName.data(), nsize)) {
            return 3;
        }

        // GameObject - ScriptName
        if (!stream->readS(&nsize, 1)) {
            return 4;
        }
        
        std::string componentName(nsize, '\0');
        if (!stream->readS(componentName.data(), nsize)) {
            return 5;
        }

        std::size_t stream_size = 0;
        if (!stream->readS(&stream_size, 1)) {
            return 6;
        }
        if (stream_size <= 0) {
            continue;
        }
        auto nextPosition = stream->tellg() + static_cast<std::streampos>(stream_size);

        // deserialize
        auto it = tempList.find(objectName);
        if (it != tempList.end()) {
            auto gameObject = it->second;


            Event::Add(&SetupScript, gameObject, stream->getPath(), stream->tellg(), componentName);
        }

        if (stream->tellg() != nextPosition)
            stream->seekg(nextPosition, std::ios_base::beg);
    }
    return 0;
}

int Model3DHandler::WriteData(Stream *stream, const GameObject *gameObject) {
    DataSegment segment = {gHeaderType_None, 0};

    std::list<std::tuple<std::string, std::string, Component *>> m_componentTemp;

    // texture
    {}

    // material
    {}
    
    // GameObject - Node
    {
        if (WriteGameObject(stream, gameObject, m_componentTemp) != 0) {
            return 1;
        }

        segment = {gHeaderType_Object, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 2;
        }
        stream->flush();
    }

    // Scripts
    {
        if (WriteScript(stream, m_componentTemp) != 0) {
            return 3;
        }

        segment = {gHeaderType_Script, static_cast<std::size_t>(stream->getDataSize())};
        if (!stream->writeS(&segment, 1, false)) {
            return 2;
        }
        stream->flush();
    }

    segment = {gHeaderType_None, 0};
    if (!stream->writeS(&segment, 1, false)) {
        return 3;
    }
    
    return 0;
}

int Model3DHandler::WriteGameObject(Stream *stream, const GameObject *gameObject, std::list<std::tuple<std::string, std::string, Component *>> &tempList) {
    // write-name
    {
        auto name = gameObject->getName();

        auto nsize = name.size();
        if (!stream->writeS(&nsize, 1)) {
            return 1;
        }

        if (!stream->writeS(name.data(), name.size())) {
            return 2;
        }
    }
    
    // write-tag
    {
        auto tag = gameObject->getTag();

        auto nsize = tag.size();
        if (!stream->writeS(&nsize, 1)) {
            return 3;
        }

        if (!stream->writeS(tag.data(), tag.size())) {
            return 4;
        }
    }

    // write-status
    {
        auto bActive = gameObject->getActive();
        if (!stream->writeS(&bActive, 1)) {
            return 5;
        }

        auto bEnable = gameObject->getEnable();
        if (!stream->writeS(&bEnable, 1)) {
            return 6;
        }
    }

    // Temp
    auto entt = gameObject->getEntity();

    for (auto &&[cname, ptr] : entt->getComponentMap()) {
        tempList.emplace_front(gameObject->getName(), cname, &(*ptr));
    }

    auto children = gameObject->getTransform()->getChildren();
    auto children_size = children.size();

    if (!stream->writeS(&children_size, 1)) {
        return 7;
    }

    int result = 0;
    for (auto &child : children) {
        result = WriteGameObject(stream, child->getGameObject(), tempList);
        if (result != 0) {
            break;
        }
    }
    return result != 0 ? (8 + result) : 0;
}

int Model3DHandler::WriteScript(Stream *stream, std::list<std::tuple<std::string, std::string, Component *>> &tempList) {
    auto size = tempList.size();

    if (!stream->writeS(&size, 1)) {
        return 1;
    }

    for (auto &&[object, component, ptr] : tempList) {
        auto nsize = object.size();

        // GameObject - name
        {
            if (!stream->writeS(&nsize, 1)) {
                return 2;
            }
            
            if (!stream->writeS(object.data(), nsize)) {
                return 3;
            }
        }

        // GameObject - ScriptName
        {
            nsize = component.size();
            if (!stream->writeS(&nsize, 1)) {
                return 4;
            }
            
            if (!stream->writeS(component.data(), nsize)) {
                return 5;
            }
        }

        // get stream size
        std::size_t stream_size = 0;
        {
            Stream temp; // 
            ptr->Serialize(&temp);
    
            stream_size = temp.getDataSize();
        }

        if (!stream->writeS(&stream_size, 1)) {
            return 6;
        }
        
        if (stream_size <= 0) {
            continue;
        }
        ptr->Serialize(stream);
    }
    return 0;
}

void WriteAnimation(const FbxScene *fbxScene) {
    auto &animations = fbxScene->getAnimations();

    FileHeader fileHeader{{'g', 'r', 'a', 'n'}, 1.0f};

    for (auto &animation : animations) {
        auto name = animation.getName();

        Stream stream(File::GetAsset("/animation/" + name + ".anm"), std::ios_base::out | std::ios_base::binary);

        stream.writeS<FileHeader>(&fileHeader, 1, false);

        gAnimationHeader pAnimationHeader;
        std::strcpy(pAnimationHeader.m_name, name.c_str());
        pAnimationHeader.m_tick = animation.getTick();
        pAnimationHeader.m_duration = animation.getDuration();

        stream.writeS<gAnimationHeader>(&pAnimationHeader, 1, false);

        auto &channels = animation.getChannels();

        grm::size sChannel = channels.size();
        gChannel *pChannels = new gChannel[sChannel];

        for (auto i=0;i<sChannel;++i) {
            auto &channel = channels[i];

            auto &name = channel.getName();
            auto &keysScale = channel.getKeysScale();
            auto &keysRotation = channel.getKeysRotation();
            auto &keysPosition = channel.getKeysPosition();

            auto &pChannel = pChannels[i];
            std::strcpy(pChannel.m_name, name.c_str());

            // gen key scale
            pChannel.m_keyScaleCount = keysScale.size();
            pChannel.m_keyScale = new gKeyScale[pChannel.m_keyScaleCount];
            for (auto i=0;i<pChannel.m_keyScaleCount;++i) {
                pChannel.m_keyScale[i].time = keysScale[i].time;
                pChannel.m_keyScale[i].value = keysScale[i].value;
            }

            //gen key rotation
            pChannel.m_keyRotationCount = keysRotation.size();
            pChannel.m_keyRotation = new gKeyRotation[pChannel.m_keyRotationCount];
            for (auto i=0;i<pChannel.m_keyRotationCount;++i) {
                pChannel.m_keyRotation[i].time = keysRotation[i].time;
                pChannel.m_keyRotation[i].value = keysRotation[i].value;
            }

            // gen key position
            pChannel.m_keyPositionCount = keysPosition.size();
            pChannel.m_keyPosition = new gKeyPosition[pChannel.m_keyPositionCount];
            for (auto i=0;i<pChannel.m_keyPositionCount;++i) {
                pChannel.m_keyPosition[i].time = keysPosition[i].time;
                pChannel.m_keyPosition[i].value = keysPosition[i].value;
            }
        }
        
        stream.writeS<grm::size>(&sChannel, 1, false);
        for (auto i=0;i<sChannel;++i) {
            auto &pChannel = pChannels[i];

            // ame
            stream.writeS<char>(pChannel.m_name, 256, false);

            // gen key scale
            stream.writeS<grm::size>(&pChannel.m_keyScaleCount, 1, false);
            stream.writeS<gKeyScale>(pChannel.m_keyScale, pChannel.m_keyScaleCount, false);

            //gen key rotation
            stream.writeS<grm::size>(&pChannel.m_keyRotationCount, 1, false);
            stream.writeS<gKeyRotation>(pChannel.m_keyRotation, pChannel.m_keyRotationCount, false);

            // gen key position
            stream.writeS<grm::size>(&pChannel.m_keyPositionCount, 1, false);
            stream.writeS<gKeyPosition>(pChannel.m_keyPosition, pChannel.m_keyPositionCount, false);
        }
        
        // ========================= CLEAN =========================
        for (auto i=0;i<sChannel;++i) {
            auto &pChannel = pChannels[i];

            // delete keys scale
            delete[] pChannel.m_keyScale;

            // delete keys rotation
            delete[] pChannel.m_keyRotation;

            // delete keys position
            delete[] pChannel.m_keyPosition;
        }
        
        delete[] pChannels;

        stream.close();
    }
}

void Model3DHandler::SetupScene(std::shared_ptr<FbxScene> scene) {
    auto& fRootNode = scene->getRootNode();

    WriteAnimation(&(*scene));
    
    for (const auto& node : fRootNode.getChildren()) {
        Threadpool::AddTask(&Model3DHandler::ConvertToGameObject, scene, node);
    }
}

void Model3DHandler::ConvertToGameObject(std::shared_ptr<FbxScene> scene, const FbxNode node) {
    std::vector<std::unique_ptr<GameObject>> m_tempObject;
    GameObject *gameObject = nullptr;
    
    CreateObjects(&(*scene), &node, nullptr, &gameObject, m_tempObject);
    if (!gameObject) {
        return Console::LogError("[ Model3DHandler ] Falied to convert fbx model to grmx.");
    } 

    ExportGRMX(File::GetAsset("/model/" + gameObject->getName() + ".gmdx"), gameObject);
}

void Model3DHandler::CreateObjects(const FbxScene *scene, const FbxNode *node, Transform *parent, GameObject **out, std::vector<std::unique_ptr<GameObject>> &tempObject) {
    auto gameObject = &(*tempObject.emplace_back(std::make_unique<GameObject>(node->getName())));
    if (!(*out)) {
        *out = gameObject;
    }

    gameObject->setTransform(gameObject->addComponent<Transform>());

    auto transform = gameObject->getTransform();
    transform->setLocalPosition(node->getPosition());
    transform->setLocalRotation(node->getRotation());
    transform->setLocalScale(node->getScale());

    if (parent) {
        transform->setParent(parent);
    }

    if (node->hasMeshe()) {
        std::unordered_map<std::string, FbxMaterial> m_materials;
        std::unordered_map<std::string, FbxBone> m_bones;
        grm::u32 verticeCount = 0;
        std::string meshName;
        Mesh m_tempMesh;

        for (auto &fbxMesh : node->getMeshes()) {
            meshName += fbxMesh.getName();

            for (auto &mIdx : fbxMesh.getMaterials()) {
                auto fbxMaterial = scene->getMaterial(mIdx);
                
                if (m_materials.find(fbxMaterial.getName()) != m_materials.end()) {
                    continue;
                }
                m_materials.emplace(fbxMaterial.getName(), fbxMaterial);
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

        auto meshFilter = gameObject->addComponent<MeshFilter>();
        if (!meshFilter) { return; }

        meshFilter->setMesh(m_mesh);

        auto meshRender = gameObject->addComponent<MeshRender>();
        if (!meshRender) { return; }

        if (!m_bones.empty()) {
            auto animator = gameObject->addComponent<Animator>();
            if (!animator) { return; }
    
            for (auto &&[name, bone] : m_bones) {
                animator->m_bones[name] = std::make_shared<Bone>(name, bone.getPosition(), bone.getScale(), bone.getRotation(), bone.getID());
            }
        }

        for (auto &&[name, fbxMaterial] : m_materials) {
            auto material = meshRender->addMaterial(fbxMaterial.getName(), fbxMaterial.getName(), fbxMaterial.getColor());

            int textureID = 0;
            for (auto &&[type, path] : fbxMaterial.getTextures()) {
                if (type != TextureType_Albedo) {
                    continue;
                }
                AssetsManager::LoadTexture(path, &Model3DHandler::SetupTextureMaterial, material, textureID);
                textureID++;
            }
        }
    }

    // escreve os filhos
    for (const auto &child : node->getChildren()) {
        CreateObjects(scene, &child, transform, out, tempObject);
    }
}

void Model3DHandler::SetupTextureMaterial(Material *material, int id, grr::gTexture *texture) {
    if (!material || !texture) {
        return;
    }

    material->setTexture(id, texture);
}








