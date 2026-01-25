#pragma once

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <gr-render/gCommon.h>

#include <unordered_set>
#include <functional>

class GameObjectSaveTask : public IAsyncTask<int>
{
public:
    GameObjectSaveTask() = delete;
    GameObjectSaveTask(const std::string &file, GameObjectID gameObject, std::function<void (GameObjectID)> func);

private:
    const std::string m_file;

    GameObjectID m_gameObjectID;

    std::function<void (GameObjectID)> m_func;

    std::vector<std::tuple<std::string, std::string, std::string>> m_textureMap;

    std::vector<std::tuple<std::string, uint64_t, const Behavior *>> m_componentdata;

    std::unordered_map<std::string, MaterialID> m_materialmap;

    std::unordered_set<MeshID> m_meshmap;

    // std::unordered_set<std::string> m_shadermap;

    int doInBackground() override;

    void onPostExecute(int result) override;

    int write_metadata(Stream *stream);

    int write_segment(Stream* stream, HeaderType_ type, std::function<int(Stream*)> func);

    int write_data(Stream *stream);

    // int write_shader(Stream *stream);

    int write_texture(Stream *stream);

    int write_material(Stream *stream);

    int write_mesh(Stream *stream);

    int write_gameobject(Stream *stream, const GameObject *go);

    int write_script(Stream *stream);

    void read_gameobject(const GameObject *gameObject = nullptr);
};




