#pragma once

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/ecs/default.hpp"
#include "gr-base/managers/default.hpp"

#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <string>

class SceneSaveTask : public IAsyncTask<int>
{
public:
    SceneSaveTask(const std::string &file, const Scene *scene, std::function<void ()> func = nullptr);

private:
    const std::string m_file;

    const Scene *m_scene;

    std::function<void ()> m_func;

    std::vector<std::tuple<std::string, std::vector<std::tuple<uint64_t, const Component *>>>> m_componentdata;

    std::unordered_map<std::string, MaterialID> m_materialmap;

    std::unordered_set<std::string> m_shadermap;

    std::unordered_set<MeshID> m_meshmap;

    int doInBackground() override;

    void onPostExecute(int result) override;

    int write_segment(Stream* stream, HeaderType type, std::function<int(Stream*)> func);

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


