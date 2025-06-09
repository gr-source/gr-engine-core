#pragma once

#include "fbxScene.hpp"

#include <assimp/cimport.h>

#include <assimp/scene.h>

#include <functional>
#include <memory>

class Filmbox
{
public:
    Filmbox();
    ~Filmbox();

    void load(const std::string &path);

    FbxScene *get_scene() const;

    std::unique_ptr<FbxScene> get_scene();

private:
    std::unique_ptr<FbxScene> m_scene;

    static void SceneLoader(const aiScene *scene, FbxScene *fbxScene);

    static void ProcessNode(aiNode *mNode, FbxNode& node, const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap);

    static void LoadMaterial(const aiScene *scene, FbxScene *fbxScene);

    static void ProcessTexture(aiMaterial *iMaterial, FbxMaterial& fbxMaterial);

    static void LoadMesh(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex);

    static void ProcessMesh(aiMesh *mMesh, FbxScene *fbxScene, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex);

    static void InserMesh(FbxMesh *fbxMesh, aiMesh *mMesh, std::unordered_map<std::string, Matrix4x4> &boneMap, std::unordered_map<std::string, int> &boneIndex);

    static void LoadAnimation(const aiScene *scene, FbxScene *fbxScene, std::unordered_map<std::string, int> &boneIndex);

    static void AddKeys(aiNodeAnim *mNodeAnim, FbxChannel *channel);
};


