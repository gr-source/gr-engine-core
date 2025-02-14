#pragma once

#include "taskStatus.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/default.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

enum SceneLoadFlags {
    SceneLoadFlags_Metadata = 1 << 1,
    SceneLoadFlags_Texture  = 1 << 2,
    SceneLoadFlags_Material = 1 << 3,
    SceneLoadFlags_Mesh     = 1 << 4,
    SceneLoadFlags_Script   = 1 << 5,
    SceneLoadFlags_Object   = 1 << 6,
    SceneLoadFlags_Load     = SceneLoadFlags_Metadata | SceneLoadFlags_Texture | SceneLoadFlags_Material | SceneLoadFlags_Mesh | SceneLoadFlags_Script | SceneLoadFlags_Object,
};

using SceneLoadFlags_ = uint8_t;

using SceneFunc = std::function<void (Scene *)>;

class SceneTask {
public:
    SceneTask(const std::string &path, SceneLoadFlags_ flags, SceneFunc func);

    void render();
    
    void addFunction(SceneFunc func);
    
    TaskStatus getStatus() const;

private:
    static std::mutex m_mutex;

    std::vector<SceneFunc> function_list;

    TaskStatus status;

    void setStatus(TaskStatus s);

    void upload(std::string path);

    void invoke(Scene *scene);

protected:
    std::unordered_map<std::string, GameObject *> tempGameObjectMap;
    
    std::vector<std::shared_ptr<GameObject>> tempGameObject;

    std::vector<GameObject *> tempRootNode;

    std::shared_ptr<GameObject> m_camera;

    std::shared_ptr<Stream> stream;

    std::unique_ptr<Scene> scene;

    std::string scene_name;
    
    void doInBackground(std::string path, SceneLoadFlags_ flags);

    int readMetadata();

    int readTexture();

    int readMaterial();

    int readMesh();

    int readObject();

    int readObjectNode(Transform *parent = nullptr, bool main = false);

    int readScript();
};



