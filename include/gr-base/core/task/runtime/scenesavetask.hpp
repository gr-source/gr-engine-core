#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

class SceneSaveTask : public IAsyncTask<int>
{
public:
    SceneSaveTask(const std::string &path, const Scene *scene, std::function<void (const Scene *)> func = nullptr);

private:
    const std::string m_path;

    const Scene *m_scene;

    std::function<void (const Scene *)> m_func;

    std::vector<std::tuple<std::string, std::string, const Component *>> m_componentdata;

    int doInBackground() override;

    void onPostExecute(int result) override;

    int write_metadata(Stream *stream);

    int write_data(Stream *stream);

    int write_shader(Stream *stream);

    int write_texture(Stream *stream);

    int write_material(Stream *stream);

    int write_mesh(Stream *stream);

    int write_gameobject(Stream *stream);

    int write_script(Stream *stream);

    bool serialize(Stream *stream, const GameObject *go);

    void read_gameobject();

    void read_data(const GameObject *gameObject);
};

/*
#include "gr-base/core/file/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <memory>
#include <string>

class SceneSaveTask
{
public:
    SceneSaveTask(const Scene *scene);

private:
    const Scene *m_scene;

    std::shared_ptr<Stream> m_stream;

protected:
    std::vector<std::tuple<std::string, std::string, Component *, BaseChunk *>> m_scriptCache;

    void doInBackground(std::string path);

    int writeData();

    int writeMetadata();

    int write_shader();

    int writeTexture();

    int writeMaterial();

    int writeMesh();

    int writeObject();

    int writeScript();

    int writeGameObject(const GameObject *go);
};
*/

