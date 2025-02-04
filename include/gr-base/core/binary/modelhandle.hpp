#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>

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

    static std::mutex mutex;

    std::unordered_map<const GameObject *, std::tuple<Material *, grr::gTexture *>> m_textures;

    std::unordered_map<const GameObject *, std::vector<std::tuple<std::string, Component *>>> m_components;

    std::unordered_map<const GameObject *, std::vector<std::tuple<std::string, Material *>>> m_materials;

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

    void generateScriptCache(const GameObject *go);

    void gen(const std::string &path, const GameObject *goGameObject) const;

    int writeData(Stream *stream, const GameObject *go) const;

    int writeTexture(Stream *stream, const GameObject *go) const;

    int writeMaterial(Stream *stream, const GameObject *go) const;

    int writeScript(Stream *stream, const GameObject *go) const;

    int writeGameObject(Stream *stream, const GameObject *go) const;

    std::vector<std::tuple<std::string, Component *>> getScriptOfRefGameObject(const GameObject *go) const;

    std::vector<std::tuple<std::string, Material *>> getMaterialOfRefGameObject(const GameObject *go) const;

    static void SetupFBX(const std::string &path, std::unique_ptr<FbxScene> scene);
};


