#pragma once

#include "gr-base/core/binary/gHeader.hpp"
#include "gr-base/core/file/default.hpp"

#include "gr-base/core/geometry/default.hpp"
#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/chunk.hpp"
#include "gr-base/ecs/behavior.hpp"
#include <functional>
#include <unordered_set>

class ChunkSaveTask : public IAsyncTask<int>
{
public:
    ChunkSaveTask(ChunkID chunkID, std::function<void (ChunkID)> func);

private:

    std::vector<std::tuple<std::string, std::vector<std::tuple<uint64_t, const Behavior *>>>> m_componentdata;

    std::unordered_map<std::string, MaterialID> m_materialmap;

    std::unordered_set<std::string> m_shadermap;

    std::unordered_set<MeshID> m_meshmap;

    std::function<void (ChunkID)> m_function;

    ChunkID m_chunkID;

    int doInBackground() override;

    void onPostExecute(int result) override;

    int write_segment(Stream* stream, HeaderType type, std::function<int(Stream*)> func);

    int write_data(Stream *stream);

    int write_shader(Stream *stream);

    int write_texture(Stream *stream);

    int write_material(Stream *stream);

    int write_mesh(Stream *stream);

    int write_gameobject(Stream *stream);

    int write_script(Stream *stream);

    int write_metadata(Stream *stream);

    void read_gameobject();

    void read_data(const GameObject *gameObject);

    bool serialize(Stream *stream, const GameObject *go);
};

