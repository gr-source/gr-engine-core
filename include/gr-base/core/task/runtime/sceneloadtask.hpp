#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

class SceneLoadTask : public IAsyncTask<int>
{
public:
    SceneLoadTask() = delete;
    SceneLoadTask(const std::string &path);

    void addFunction(std::function<void (Scene *)> func);

private:
    const std::string m_path;

    std::vector<std::function<void (Scene *)>> m_funcList;

    std::vector<GameObject *> m_rootList;

    std::unordered_map<std::string, GameObject *> m_gameobjectmap;

    std::string m_name;

    int doInBackground() override;

    void onPostExecute(int result) override;

    int read_metadata(Stream *stream);

    int read_shader(Stream *stream);

    int read_material(Stream *stream);

    int read_texture(Stream *stream);

    int read_mesh(Stream *stream);

    int read_gameobject(Stream *stream);

    int read_script(Stream *stream);

    GameObject *deserialize(Stream *stream, Transform *parent = nullptr);
};

/*
#include "default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/default.hpp"
#include "gr-base/core/scene.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

enum SceneLoadFlags {
    SceneLoadFlags_Metadata = 1 << 1,
    SceneLoadFlags_Shader   = 1 << 2,
    SceneLoadFlags_Texture  = 1 << 3,
    SceneLoadFlags_Material = 1 << 4,
    SceneLoadFlags_Mesh     = 1 << 5,
    SceneLoadFlags_Script   = 1 << 6,
    SceneLoadFlags_Object   = 1 << 7,
    SceneLoadFlags_Load     = SceneLoadFlags_Metadata | SceneLoadFlags_Shader | SceneLoadFlags_Texture | SceneLoadFlags_Material | SceneLoadFlags_Mesh | SceneLoadFlags_Script | SceneLoadFlags_Object,
};

using SceneLoadFlags_ = uint8_t;

using SceneFunc = std::function<void (Scene *)>;

class SceneLoadTask
{
public:
    SceneLoadTask(const std::string &path, SceneLoadFlags_ flags, SceneFunc func);

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

    int read_shader();

    int readTexture();

    int readMaterial();

    int readMesh();

    int readObject();

    int readObjectNode(Transform *parent = nullptr, bool main = false);

    int readScript();
};
*/


