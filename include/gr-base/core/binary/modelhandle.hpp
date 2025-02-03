#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/filmbox/default.hpp"

#include "gr-base/ecs/gameObject.hpp"

class ModelHandle {
public:
    static void ImportFBX(const std::string &path);

    static void Render();

    ModelHandle(std::shared_ptr<FbxScene> scene);

private:
    std::unordered_map<const GameObject *, std::vector<std::tuple<std::string, Component *>>> m_components;
    std::unordered_map<const GameObject *, std::vector<std::tuple<std::string, Material *>>> m_materials;

    std::vector<std::unique_ptr<GameObject>> gameObjects;

    std::unordered_map<const FbxNode *, bool> m_active;

    std::shared_ptr<FbxScene> m_scene;

    void render();

    void renderNode(const FbxNode *node);

    void enableNode(const FbxNode *node);

    void convertToGameObject();

    GameObject *CreateGameObject(const FbxNode *node, Transform *parent = nullptr);

    void generateMesh(const FbxNode *node, GameObject *go);

    void generateScriptCache(const GameObject *go);

    void build(std::vector<GameObject *> gos);

    void gen(const std::string &path, const GameObject *goGameObject) const;

    int writeData(Stream *stream, const GameObject *go) const;

    int writeTexture(Stream *stream) const;

    int writeMaterial(Stream *stream, const GameObject *go) const;

    int writeScript(Stream *stream, const GameObject *go) const;

    int writeGameObject(Stream *stream, const GameObject *go) const;

    std::vector<std::tuple<std::string, Component *>> getScriptOfRefGameObject(const GameObject *go) const;

    std::vector<std::tuple<std::string, Material *>> getMaterialOfRefGameObject(const GameObject *go) const;

    static void SetupFBX(std::shared_ptr<FbxScene> scene);

    static std::vector<std::unique_ptr<ModelHandle>> taskList;
};


