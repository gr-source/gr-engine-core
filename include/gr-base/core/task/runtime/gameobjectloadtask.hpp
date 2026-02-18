#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gCommon.h>

#include "gr-base/core/default.hpp"

#include "gr-base/core/geometry/material.hpp"
#include "gr-base/core/geometry/mesh.hpp"

#include <functional>

class GameObjectLoadTask : public IAsyncTask<int>
{
public:
    GameObjectLoadTask(const std::string &path, std::function<void (GameObjectHandle )> func);

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

    std::function<void (GameObjectHandle)> m_func;

    std::unique_ptr<GameObjectCache> m_root;

    std::vector<MaterialDataCahe> m_materialCacheList;

    std::vector<MeshDataCahe> m_meshCacheList;

    GameObjectHandle m_gameObject;

    std::unordered_map<std::string, GameObjectCache*> m_gameobjectmap;

    int doInBackground() override;

    void onPostExecute(int result) override;

    void CreateMaterialFromCache();

    void CreateMeshFromCache();

    GameObjectHandle CreateFromCache(GameObjectCache* cache);

    int read_metadata(Stream *stream);

    int read_data(Stream *stream);

    // int read_shader(Stream *stream);

    int read_texture(Stream *stream);

    int read_material(Stream *stream);

    int read_mesh(Stream *stream);

    std::unique_ptr<GameObjectCache> read_gameobject(Stream *stream);

    int read_script(Stream *stream);

    int read_script_from_cache(GameObject* gameObject, Stream *stream);
};



/*
#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

#include "default.hpp"


using GameObjectFunc = std::function<void (GameObject *)>;

class GameObjectLoadTask
{
public:
    GameObjectLoadTask(const std::string &path, GameObjectFunc func);

private:
    std::unordered_map<std::string, GameObject *> m_tempGameObjectMap;
    
    std::vector<std::shared_ptr<GameObject>> m_tempGameObject;

    std::shared_ptr<Stream> m_stream;

    GameObjectFunc m_function;

    GameObject *m_rootNode;

    void upload(std::string path);

protected:
    void doInBackground(std::string path);

    int readData();

    int readGameObject(Transform *parent = nullptr);

    int readTexture();

    int readMaterial();

    int readScript();

};
*/




