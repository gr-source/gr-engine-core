#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-ecs/behavior.hpp>

#include <unordered_map>
#include <unordered_set>
#include <functional>

class SceneSaveTask : public IAsyncTask<int>
{
public:
    SceneSaveTask(const std::string &path, const Scene *scene, std::function<void (const Scene *)> func = nullptr);

private:
    const std::string m_path;

    const Scene *m_scene;

    std::function<void (const Scene *)> m_func;

    std::vector<std::tuple<std::string, uint64_t, const Behavior *>> m_componentdata;

    std::unordered_map<std::string, MaterialID> m_materialmap;

    std::unordered_set<std::string> m_shadermap;

    std::unordered_set<MeshID> m_meshmap;

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


