#pragma once

#include "gr-base/core/thread/asynctask.hpp"

#include "gr-base/asset/modelasset.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <functional>
#include <string>

class ModelLoadTask : public IAsyncTask<ModelAsset>
{
public:
    template <typename ...Args>
    ModelLoadTask(const std::string& file, Scene* scene, Args&&... args);

private:
    std::function<void (Handle)> m_func;

    std::string m_file;

    Scene* m_scene;

    void onPreExecute() override;

    ModelAsset doInBackground() override;

    void onPostExecute(Result<ModelAsset> result) override;

    Entity CreateFromAsset(const ModelAsset* asset);
};

template <typename ...Args>
inline ModelLoadTask::ModelLoadTask(const std::string& file, Scene* scene, Args&&... args) : m_file(file), m_func(nullptr), m_scene(scene)
{
    if constexpr (sizeof...(Args) > 0)
        m_func = std::bind(std::forward<Args>(args)..., std::placeholders::_1);
}


