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
    ModelLoadTask(const std::string& file, Scene* scene, std::function<void (Entity)> func);

private:
    std::function<void (Handle)> m_func;

    std::string m_file;

    Scene* m_scene;

    void onPreExecute() override;

    ModelAsset doInBackground() override;

    void onPostExecute(Result<ModelAsset> result) override;

    Entity CreateFromAsset(const ModelAsset* asset);
};


