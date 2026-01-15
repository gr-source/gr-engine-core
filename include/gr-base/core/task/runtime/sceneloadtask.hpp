#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gCommon.h>

#include <functional>

class SceneLoadTask : public IAsyncTask<Scene*>
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

    std::unordered_map<std::string, grr::gShader *> m_shadermap;

    std::unordered_map<std::string, MaterialID> m_materialmap;

    std::string m_name;

    Scene* doInBackground() override;

    void onPostExecute(Scene* result) override;

    int read_metadata(Stream *stream);

    int read_shader(Stream *stream);

    int read_texture(Stream *stream);

    int read_material(Stream *stream);

    int read_mesh(Stream *stream);

    int read_gameobject(Stream *stream);

    int read_script(Stream *stream);

    GameObject *deserialize(Stream *stream, Transform *parent = nullptr);
};

