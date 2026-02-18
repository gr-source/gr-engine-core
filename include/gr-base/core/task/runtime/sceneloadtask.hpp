#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gCommon.h>

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

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

        bool IsStatic;

        std::vector<uint8_t> buffer;

        std::vector<std::unique_ptr<GameObjectCache>> chindren;
    };


    struct MeshDataCahe
    {
        std::string name;
        Mesh mesh;
    };

    struct MaterialDataCahe
    {
        std::string name;
        Material material;
    };

    const std::string m_path;

    std::vector<std::function<void (Scene *)>> m_funcList;

    std::vector<std::unique_ptr<GameObjectCache>> m_rootList;

    std::vector<MaterialDataCahe> m_materialCacheList;

    std::vector<MeshDataCahe> m_meshCacheList;

    std::unordered_map<std::string, GameObjectCache*> m_gameobjectmap;

    std::unordered_map<std::string, grr::gShader *> m_shadermap;

    std::string m_name;

    int doInBackground() override;

    void onPostExecute(int result) override;

    void CreateMaterialFromCache();

    void CreateMeshFromCache();

    GameObjectHandle CreateFromCache(GameObjectCache* cache);

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

