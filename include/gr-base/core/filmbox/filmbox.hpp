#pragma once

#include "fbxScene.hpp"

#include <assimp/cimport.h>

#include <assimp/scene.h>

#include <functional>
#include <memory>

class Filmbox {
public:
    Filmbox() = delete;
    ~Filmbox() = delete;

    static void Load(const std::string &path, std::function<void(std::unique_ptr<FbxScene>)> func, bool bCallInMainThread = true);

private:
    static void SceneLoader(const aiScene *scene, FbxScene *fbxScene);

    static void ProcessNode(aiNode *mNode, FbxNode& node, const aiScene *scene, FbxScene *fbxScene);

    static void LoadMaterial(const aiScene *scene, FbxScene *fbxScene);

    static void ProcessTexture(aiMaterial *iMaterial, FbxMaterial& fbxMaterial);

    static void LoadMesh(const aiScene *scene, FbxScene *fbxScene);

    static void ProcessMesh(aiMesh *mMesh, FbxScene *fbxScene);

    static void InserMesh(FbxMesh *fbxMesh, aiMesh *mMesh);

    static void LoadAnimation(const aiScene *scene, FbxScene *fbxScene);

    static void AddKeys(aiNodeAnim *mNodeAnim, FbxChannel &channel);
};


