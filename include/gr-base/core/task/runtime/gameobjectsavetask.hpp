#pragma once

#include "gr-base/core/geometry/default.hpp"

#include "gr-base/core/file/default.hpp"

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/ecs/default.hpp"

#include <gr-render/gCommon.h>

class GameObjectSaveTask : public IAsyncTask<int>
{
public:
    GameObjectSaveTask() = delete;
    GameObjectSaveTask(const std::string &dir, const GameObject *gameObject, std::function<void (const GameObject *)> func);

private:
    const std::string m_path;

    const GameObject *m_gameObject;

    std::function<void (const GameObject *)> m_func;

    std::vector<std::tuple<std::string, std::string, std::string>> m_textureMap;

    std::vector<std::tuple<std::string, std::string, Material *>> m_materialMap;

    std::unordered_map<std::string, std::tuple<std::string, grr::gShader *>> m_shaderMap;

    std::vector<std::tuple<std::string, std::string, Component *>> m_scriptMap;

    int doInBackground() override;

    void onPostExecute(int result) override;

    int write_data();

    int write_gameobject(const GameObject *go);

    int write_shader();

    int write_texture();

    int write_material();

    int write_script();

    void remape_gameobject(const GameObject *go);
};




