#include "gr-base/core/filmbox/filmbox.hpp"

#include "gr-base/core/file/file.hpp"

#include "gr-base/core/assetsManager.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/event.hpp"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <gr-math/matrix4x4.hpp>
#include <gr-math/quaternion.hpp>
#include <gr-math/vector3.hpp>
#include <string>
#include <unordered_map>

#define NORMALIZE(v) if (std::abs(v.x) > 6.0f) v.x /= 1e2f; if (std::abs(v.y) > 6.0f) v.y /= 1e2f; if (std::abs(v.z) > 6.0f) v.z /= 1e2f;

#define NORM(n) n > 6.0f ? n / 1e2f : n

void Filmbox::Load(const std::string &path, std::function<void(std::unique_ptr<FbxScene>)> func, bool bCallInMainThread) {
    if (!File::Exists(path)) {
        return Console::LogError("[ Filmbox ] Falied open file: %s", path);
    }

    Assimp::Importer importer;

    auto scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData | aiProcess_Triangulate);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        return Console::LogError("[ Filmbox ] Falied to load scene: %s", importer.GetErrorString());
    }

    auto m_fbxScene = std::make_unique<FbxScene>(scene->mName.C_Str());

    SceneLoader(scene, &(*m_fbxScene));

    importer.FreeScene();

    Console::LogInfo("[ Filmbox ] loaded sucess full.");

    if (!func) {
        return Console::LogWarning("[ Filmbox ] Callfunction is not valid.");
    }

    // if (bCallInMainThread)
    //     return Event::Add(func, std::move(m_fbxScene));

    func(std::move(m_fbxScene));
}

void Filmbox::SceneLoader(const aiScene *scene, FbxScene *fbxScene) {
    if (scene->HasMaterials()) {
        Console::log(Log::WARNING, "Loading material...");
        LoadMaterial(scene, fbxScene);
    }

    std::unordered_map<std::string, Matrix4x4> boneMap;
    std::unordered_map<std::string, int> boneIndex;

    if (scene->HasMeshes()) {
        Console::log(Log::WARNING, "Loading Mesh...");
        LoadMesh(scene, fbxScene, boneMap, boneIndex);
    }

    if (scene->HasAnimations()) {
        Console::log(Log::WARNING, "Loading Animations...");
        LoadAnimation(scene, fbxScene, boneIndex);
    }

    Console::log(Log::WARNING, "Loading Node...");

    auto mRootNode = scene->mRootNode;
    auto& fRootNode = fbxScene->m_rootNode;

    for (auto i=0;i<mRootNode->mNumChildren;++i) {
        ProcessNode(mRootNode->mChildren[i], fRootNode, scene, fbxScene, boneMap);
    }
}

void Filmbox::ProcessNode(aiNode *mNode, FbxNode& node, const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap) {
    auto name = mNode->mName;

    auto& newNode = node.addNode(name.C_Str(), nullptr);

    aiVector3D scale;
    aiVector3D position;
    aiQuaternion rotation;
    mNode->mTransformation.Decompose(scale, rotation, position);

    /*NORMALIZE(scale)*/

    // magnitude = std::sqrt((position.x * position.x) + (position.y * position.y) + (position.z * position.z));
    // if (magnitude > 1e-5f) {
    //     position.x /= magnitude;
    //     position.y /= magnitude;
    //     position.z /= magnitude;
    // }
    
    newNode.setPosition({position.x, position.y, position.z});
    newNode.setScale({scale.x, scale.y, scale.z});
    newNode.setRotation({rotation.w, rotation.x, rotation.y, rotation.z});
    
    if (boneMap.find(name.C_Str()) != boneMap.end())
    {
        Vector3 t;
        Quaternion r;
        Vector3 s;

        boneMap[name.C_Str()].decompose(s, r, t);
        
        newNode.setPosition(t);
        newNode.setRotation(r);
        newNode.setScale(s);
    }

    for (auto i=0;i<mNode->mNumMeshes;++i) {
        auto id = mNode->mMeshes[i];

        newNode.addMesh(fbxScene->getMesh(id));
    }

    for (auto i=0;i<mNode->mNumChildren;++i) {
        auto child = mNode->mChildren[i];

        ProcessNode(child, newNode, scene, fbxScene, boneMap);
    }
}

void Filmbox::LoadMaterial(const aiScene *scene, FbxScene *fbxScene) {
    for (auto i=0;i<scene->mNumMaterials;++i) {
        auto vMaterial = scene->mMaterials[i];

        auto name = vMaterial->GetName();

        auto& material = fbxScene->addMaterial(name.C_Str());
        ProcessTexture(vMaterial, material);

        material.ao = 1.0f;
        
        aiColor3D diffuse;
        if (vMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS) {
            material.albedo = { diffuse.r, diffuse.g, diffuse.b, 1.0f };
        }

        float value = 0.0f;
        if (vMaterial->Get(AI_MATKEY_REFLECTIVITY, value) == AI_SUCCESS) {
            material.metallic = value;
        }

        if (vMaterial->Get(AI_MATKEY_SHININESS, value) == AI_SUCCESS) {
            material.roughness = NORM(value);
        }
    }
}

void Filmbox::ProcessTexture(aiMaterial *iMaterial, FbxMaterial &fbxMaterial) {
    auto count = iMaterial->GetTextureCount(aiTextureType_DIFFUSE);
    for (auto i=0;i<count;++i) {
        aiString path;
        if (iMaterial->GetTexture(aiTextureType_DIFFUSE, i, &path) != AI_SUCCESS) {
            continue;
        }

        fbxMaterial.addTexture(TextureType_Albedo, path.C_Str());
        break;
    }
}

void Filmbox::LoadMesh(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex) {
    for (auto i=0;i<scene->mNumMeshes;++i) {
        ProcessMesh(scene->mMeshes[i], fbxScene, boneMap, boneIndex);
    }
}

void Filmbox::ProcessMesh(aiMesh *mMesh, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex) {
    auto name = std::string(mMesh->mName.C_Str());

    auto mesh = fbxScene->addMesh(name);

    InserMesh(mesh, mMesh, boneMap, boneIndex);
}

void Filmbox::InserMesh(FbxMesh *fbxMesh, aiMesh *mMesh, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex) {
    auto bHasPosition = mMesh->HasPositions();
    auto bHasNormal = mMesh->HasNormals();
    auto bHasTexCoord = mMesh->HasTextureCoords(0);

    fbxMesh->setMaterialIndex(mMesh->mMaterialIndex);

    for (auto i=0;i<mMesh->mNumVertices;++i) {
        auto& vertice = fbxMesh->addVertice(Vector3::zero, Vector3::zero, Vector2::zero, 0);

        if (bHasPosition) {
            auto &mVertice = mMesh->mVertices[i];

            vertice.position = {mVertice.x, mVertice.y, mVertice.z};
        }

        if (bHasNormal) {
            auto &mNormal = mMesh->mNormals[i];

            vertice.normal = {mNormal.x, mNormal.y, mNormal.z};
        }

        if (bHasTexCoord) {
            auto &mTexCoord = mMesh->mTextureCoords[0][i];

            vertice.texCoord = {mTexCoord.x, mTexCoord.y};
        }
    }

    /*
    if (mMesh->HasFaces()) {
        for (auto i = 0; i < mMesh->mNumFaces; ++i) {
            auto &mFace = mMesh->mFaces[i];

            for (auto j = 0; j < mFace.mNumIndices; ++j) {
                fbxMesh->addIndice(fbxMesh->getVertices().size() + mFace.mIndices[j]);
            }
        }
    }
    */

    if (mMesh->HasBones()) {
        for (auto i=0;i<mMesh->mNumBones;++i) {
            auto mBone = mMesh->mBones[i];

            auto name = mBone->mName;

            if (boneIndex.find(name.C_Str()) == boneIndex.end())
            {
                boneIndex.emplace(name.C_Str(), i);
            }

            // if (fbxMesh->findBone(name.C_Str()) != nullptr) {
            //     continue;
            // }

            aiVector3D scale;
            aiQuaternion rotation;
            aiVector3D position;
            mBone->mOffsetMatrix.Decompose(scale, rotation, position);

            auto& fBone = fbxMesh->addBone(name.C_Str(), i);
            fBone.setScale({scale.x, scale.y, scale.z});
            fBone.setPosition({position.x, position.y, position.z});
            fBone.setRotation({rotation.w, rotation.x, rotation.y, rotation.z});

            int parentIndex = 0;
            if (mBone->mNode->mParent != nullptr)
            {
                auto it = boneIndex.find(mBone->mNode->mParent->mName.C_Str());
                if (it != boneIndex.end())
                    parentIndex = it->second;
            }

            fBone.setParentIndex(parentIndex);

            if (boneMap.find(name.C_Str()) == boneMap.end())
            {
                boneMap.emplace(name.C_Str(), fBone.getLocalMatrix());
            }

            for (auto j=0;j<mBone->mNumWeights;++j) {
                auto weight = mBone->mWeights[j];

                auto& vertice = fbxMesh->getVertice(weight.mVertexId);
                for (auto k=0;k<4;++k) {
                    if (vertice.boneIndex[k] != -1 || weight.mWeight <= 0.0f) {
                        continue;
                    }
                    vertice.weight[k] = static_cast<float>(weight.mWeight);
                    vertice.boneIndex[k] = i;
                    break;
                }
            }
        }
    }
}

void Filmbox::LoadAnimation(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, int> &boneIndex)
{
    for (auto i=0;i<scene->mNumAnimations;i++) {
        auto mAnimation = scene->mAnimations[i];

        auto name = mAnimation->mName;
        auto duration = mAnimation->mDuration;
        auto tick = mAnimation->mTicksPerSecond;

        auto &animation = fbxScene->addAnimation(name.C_Str(), duration, tick);
        animation.setChannel(std::vector<FbxChannel>(boneIndex.size(), {"None"}));

        for (auto j=0;j<mAnimation->mNumChannels;j++) {
            auto mChannel = mAnimation->mChannels[j];

            auto cName = mChannel->mNodeName;

            auto it = boneIndex.find(cName.C_Str());
            if (it == boneIndex.end())
            {
                continue;
            }

            auto channel = animation.getChannel(it->second);
            channel->setName(cName.C_Str());

            AddKeys(mChannel, channel);
        }
    }
}

void Filmbox::AddKeys(aiNodeAnim *mNodeAnim, FbxChannel *channel) {
    for (auto i=0;i<mNodeAnim->mNumPositionKeys;++i) {
        auto key = mNodeAnim->mPositionKeys[i];
        channel->addKeyPosition(static_cast<float>(key.mTime), Vector3{key.mValue.x, key.mValue.y, key.mValue.z});
    }

    for (auto i=0;i<mNodeAnim->mNumScalingKeys;++i) {
        auto key = mNodeAnim->mScalingKeys[i];
        channel->addKeyScale(static_cast<float>(key.mTime), Vector3{key.mValue.x, key.mValue.y, key.mValue.z});
    }

    for (auto i=0;i<mNodeAnim->mNumRotationKeys;++i) {
        auto key = mNodeAnim->mRotationKeys[i];
        channel->addKeyRotation(static_cast<float>(key.mTime), Quaternion(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z));
    }
}




