#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <list>

#include "gr-base/core/filmbox/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <gr-render/gRender.h>

class Stream;
class Material;

class Model3DHandler {
public:
    Model3DHandler() = delete;
    ~Model3DHandler() = delete;

    static void ImportFBX(const std::string &path);

    static void ExportGRMX(const std::string &path, const GameObject *gameObject);
    
    static void ImportGRMX(const std::string &path);

private:
    static std::mutex m_mutex;
    
    static void LoadGRMX(const std::string &path);
    
    static int ReadData(Stream *stream);

    static int ReadGameObject(Stream *stream, Transform *parent, GameObject **gameObject, std::unordered_map<std::string, GameObject *> &tempList);

    static int ReadScript(Stream *stream, std::unordered_map<std::string, GameObject *> &tempList);

    static int WriteData(Stream *stream, const GameObject *gameObject);

    static int WriteMaterial();
    
    static int WriteGameObject(Stream *stream, const GameObject *gameObject, std::list<std::tuple<std::string, std::string, Component *>> &tempList);

    static int WriteScript(Stream *stream, std::list<std::tuple<std::string, std::string, Component *>> &tempList);

    static void SetupScene(std::shared_ptr<FbxScene> scene);

    static void ConvertToGameObject(std::shared_ptr<FbxScene> scene, const FbxNode node);

    static void CreateObjects(const FbxScene *scene, const FbxNode *node, Transform *parent, GameObject **gameObject, std::vector<std::unique_ptr<GameObject>> &tempObject);

    static void SetupTextureMaterial(Material *material, int id, grr::gTexture *texture);
};



