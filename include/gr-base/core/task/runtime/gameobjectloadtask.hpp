#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

#include "gr-base/script/default.hpp"

#include "gr-base/core/geometry/default.hpp"

#include <gr-render/gCommon.h>

class GameObjectLoadTask : public IAsyncTask<int>
{
public:
    GameObjectLoadTask(const std::string &path, std::function<void (GameObject *)> func);

private:
    const std::string m_path;

    std::function<void (GameObject *)> m_func;

    GameObject *m_gameObject;

    std::unordered_map<std::string, GameObject *> m_gameobjectmap;

    std::unordered_map<std::string, grr::gShader *> m_shadermap;

    std::unordered_map<std::string, Material *> m_materialmap;

    int doInBackground();

    void onPostExecute(int result);

    int read_metadata(Stream *stream);

    int read_data(Stream *stream);

    int read_shader(Stream *stream);

    int read_texture(Stream *stream);

    int read_material(Stream *stream);

    int read_gameobject(Stream *stream, Transform *parent = nullptr);

    int read_script(Stream *stream);
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




