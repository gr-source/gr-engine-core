#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gCommon.h>

#include <functional>

class SceneLoadTask : public IAsyncTask<int>
{
public:
    SceneLoadTask() = delete;
    SceneLoadTask(const std::string &path);

    void addFunction(std::function<void (Scene *)> func);

private:
    struct GameObjectCache
    {
        std::string name;

        std::vector<uint8_t> buffer;

        std::vector<std::unique_ptr<GameObjectCache>> chindren;
    };

    const std::string m_path;

    std::vector<std::function<void (Scene *)>> m_funcList;

    std::vector<std::unique_ptr<GameObjectCache>> m_rootList;

    std::unordered_map<std::string, GameObjectCache*> m_gameobjectmap;

    std::unordered_map<std::string, grr::gShader *> m_shadermap;

    std::unordered_map<std::string, MaterialID> m_materialmap;

    std::string m_name;

    int doInBackground() override;

    void onPostExecute(int result) override;

    GameObjectID CreateFromCache(GameObjectCache* cache);

    int read_metadata(Stream *stream);

    int read_shader(Stream *stream);

    int read_texture(Stream *stream);

    int read_material(Stream *stream);

    int read_mesh(Stream *stream);

    int read_gameobject(Stream *stream);

    int read_script(Stream *stream);

    int read_script_from_cache(GameObject* gameObject, Stream* stream);

    std::unique_ptr<GameObjectCache> deserialize(Stream *stream);
};

