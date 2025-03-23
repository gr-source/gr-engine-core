#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>

#include "gr-base/core/filmbox/fbxScene.hpp"
#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/filmbox/default.hpp"

#include "gr-base/ecs/gameObject.hpp"

#include <gr-render/gCommon.h>

class ModelHandle {
public:
    static void ImportFBX(const std::string &path);

    static void Render();

    ModelHandle(const std::string &name, std::unique_ptr<FbxScene> scene);

    void run();

private:
    static std::unordered_map<std::string, std::unique_ptr<ModelHandle>> taskListMap;

    static std::vector<ModelHandle *> taskList;

    static std::atomic<bool> bHasTasks;

    static std::mutex mutex;

    std::unordered_map<const GameObject *, bool> mModelCanImport;

    std::vector<std::unique_ptr<GameObject>> gameObjects;

    std::vector<GameObject *> rootNode;

    std::string name;

    bool isCanRender;

    void render();

    void renderNode(const GameObject *go);

    void convertToGameObject(FbxScene *scene);

    void enableNode(const GameObject *go);

    GameObject *CreateGameObject(const FbxScene *scene, const FbxNode *node, Transform *parent = nullptr);

    void generateMesh(const FbxScene *scene, const FbxNode *node, GameObject *go);

    void generateAnimation(FbxScene *fbxScene);

    static void SetupFBX(const std::string &path, std::unique_ptr<FbxScene> scene);
};


